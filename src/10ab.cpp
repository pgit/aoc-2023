//
// https://adventofcode.com/2023/day/10
//
#include <set>
#include <sstream>
#include <string>
using namespace std::literals;

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

struct Pos
{
   int x = -1, y = -1;
   bool operator==(const Pos& r) const = default;
   Pos operator+(const Pos& r) const { return Pos{x + r.x, y + r.y}; }
   const Pos& operator+=(const Pos& r)
   {
      x += r.x;
      y += r.y;
      return *this;
   }
};

struct Pipe
{
   char symbol;
   std::vector<Pos> connections;
   std::string unicodeSymbol;
   bool visited = false;
   auto operator<=>(const Pipe& other) const noexcept { return symbol <=> other.symbol; }
   auto operator<=>(char symbol) const noexcept { return this->symbol <=> symbol; }
   bool operator==(char symbol) const noexcept { return this->symbol == symbol; };
};

//
// L is a 90-degree bend connecting north and east.
// J is a 90-degree bend connecting north and west.
// 7 is a 90-degree bend connecting south and west.
// F is a 90-degree bend connecting south and east.
//
const std::set<Pipe, std::less<>> PIPES = {
   // clang-format off
   {'-', {{{-1,  0}, { 1,  0}}}, "─"},
   {'|', {{{ 0, -1}, { 0,  1}}}, "│"},
   {'L', {{{ 0, -1}, { 1,  0}}}, "└"},
   {'J', {{{ 0, -1}, {-1,  0}}}, "┘"},
   {'7', {{{ 0,  1}, {-1,  0}}}, "┐"},
   {'F', {{{ 0,  1}, { 1,  0}}}, "┌"},
   {'.', {},                     "."},
   {'I', {},                     "\x1b[44m \x1b[0m"},
   {'O', {},                     "O"},
   {'S', {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}}, "\x1b[1;31mS\x1b[0m"},
   // clang-format on
};

const std::array<Pos, 4> AROUND = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};

const Pipe DOT = *PIPES.find('.');

struct Map
{
   explicit Map(std::ifstream file)
   {
      for (std::string line; std::getline(file, line);)
         map.emplace_back(line | transform([](char c) { return *PIPES.find(c); }) |
                          to<std::vector>);
      width = map[0].size();
      height = map.size();

      for (int y = 0; y < height; ++y)
         for (int x = 0; x < height; ++x)
            if (at({x, y}) == 'S')
               start = Pos(x, y);
   }

   std::vector<std::vector<Pipe>> map;
   Pos start;
   int width;
   int height;

   Pipe& at(const Pos& c)
   {
      if (c.y >= 0 && c.y < map.size())
         if (c.x >= 0 && c.x <= map[c.y].size())
            return map[c.y][c.x];

      static Pipe mydot = DOT;
      return mydot; // yes I know I'm returning a non-const reference to a static here
   }

   bool isConnected(const Pos& a, const Pos& b)
   {
      for (auto ca : at(a).connections)
         for (auto cb : at(b).connections)
            if (a + ca == b && b + cb == a)
               return true;
      return false;
   }

   void dump() const
   {
      for (auto& row : map)
      {
         for (auto& cell : row)
            if (cell.visited)
               fmt::print("\x1b[1;32m{}\x1b[0m", cell.unicodeSymbol);
            else
               fmt::print("{}", cell.unicodeSymbol);
         fmt::println("");
      }
   }
};

int main(int argc, char* argv[])
{
   Map map(input(argc, argv));

   std::vector<Pos> track{};
   auto pos = map.start;
   for (;;)
   {
      bool dead_end = true;
      for (auto con : map.at(pos).connections)
      {
         auto next = pos + con;
         if (map.isConnected(pos, next))
         {
            if (!track.empty() && track.back() == next)
               continue;
            map.at(pos).visited = true;
            track.emplace_back(pos);
            pos = next;
            dead_end = false;
            break;
         }
      }
      if (pos == map.start)
         break;
   };

   //
   // Clear out any garbage, i.e. any pipe symbols that are not part of the loop
   //
   for (auto& row : map.map)
      for (auto& cell : row)
         if (!cell.visited)
            cell = DOT;

   //
   // Scan rows: Any encounter of '|', "┌┄┘" or "└┄┐" toggles the inside flag, and any encounter
   // of '.' while the flag is true means that the '.' is inside the loop.
   //
   // This loop expectes the map to be 'clean', i.e. contain only the (closed) loop and '.'.
   //
   int count = 0;
   for (auto& row : map.map)
   {
      bool inside = false;
      for (auto it = row.begin(); it != row.end(); ++it)
      {
         switch (it->symbol)
         {
         case '|':
            inside = !inside;
            break;

         case '.':
            if (inside)
            {
               *it = *PIPES.find('I');
               count++;
            }
            break;

         case 'F':
         case 'L':
         {
            auto start = *it++;
            while (it->symbol == '-') // skip ┄
               it++;
            if (start.symbol == 'F' && it->symbol == 'J' || // ┌┄┘
                start.symbol == 'L' && it->symbol == '7') // └┄┐
               inside = !inside;
            break;
         }

         default:
            assert(false);
         }
      }
   }

   map.dump();
   fmt::println("A {} / 2 = {}", track.size(), track.size() / 2);
   fmt::println("B {}", count);
}
