//
// https://adventofcode.com/2023/day/10
//
#include <set>
#include <string>
using namespace std::literals;

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

struct Coord
{
   int x = -1, y = -1;
   bool operator==(const Coord& r) const = default;
   Coord operator+(const Coord& r) const { return Coord{x + r.x, y + r.y}; }
   const Coord& operator+=(const Coord& r)
   {
      x += r.x;
      y += r.y;
      return *this;
   }
};

struct Tile
{
   char symbol;
   std::vector<Coord> connections;
   std::string unicodeSymbol;
   bool visited = false;
   auto operator<=>(const Tile& other) const noexcept { return symbol <=> other.symbol; }
   auto operator<=>(char symbol) const noexcept { return this->symbol <=> symbol; }
   bool operator==(char symbol) const noexcept { return this->symbol == symbol; };
};

//
// L is a 90-degree bend connecting north and east.
// J is a 90-degree bend connecting north and west.
// 7 is a 90-degree bend connecting south and west.
// F is a 90-degree bend connecting south and east.
//
const std::set<Tile, std::less<>> PIPES = {
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
   {'X', {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}}, "\x1b[1;31mX\x1b[0m"},
   // clang-format on
};

const std::array<Coord, 4> AROUND = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};

const Tile DOT = *PIPES.find('.');

struct Map
{
   explicit Map(std::ifstream file)
   {
      for (std::string line; std::getline(file, line);)
         map.emplace_back(line | transform([](char c) { return *PIPES.find(c); }) |
                          to<std::vector>);

      for (auto [row, y] : zip(map, ints))
         for (auto [cell, x] : zip(row, ints))
            if (cell.symbol == 'S')
               start = Coord(x, y);
   }

   std::vector<std::vector<Tile>> map;
   Coord start;

   Tile& at(const Coord& c)
   {
      static Tile mydot = DOT;
      if (c.y >= 0 && c.y < map.size())
         if (c.x >= 0 && c.x <= map[c.y].size())
            return map[c.y][c.x];
      return mydot; // yes I know I'm returning a non-const reference to a static here
   }

   bool isConnected(const Coord& a, const Coord& b)
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

   assert(map.start.x != -1 && map.start.y != -1);
   fmt::println("start at ({}, {})", map.start.x, map.start.y);

   auto pos = map.start;
   std::vector<Coord> track{};
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

      if (dead_end || pos == map.start)
         break;
   };

   map.dump();
   fmt::println("A {} / 2 = {}", track.size(), track.size() / 2);

   //
   // Clear out any garbage, i.e. any pipe symbols that are not part of the loop
   //
   for (auto& row : map.map)
      for (auto& cell : row)
         if (!cell.visited)
            cell = DOT;

   //
   // Replace 'S' with correct symbol, so scan line algorithm works:
   //
   for (auto& pipe : PIPES)
   {
      map.at(map.start) = pipe;
      if (map.isConnected(map.start, track[1]) && map.isConnected(map.start, track.back()))
         break;
   }

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
         }
         break;

         default:
            fmt::println("unexpected symbol '{}'", it->symbol);
            assert(false);
         }
      }
   }

   map.dump();
   fmt::println("B {}", count);
}
