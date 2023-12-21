//
// https://adventofcode.com/2023/day/17
//
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

struct Coord
{
   int x = -1, y = -1;
   Coord operator-() const { return Coord{-x, -y}; }
   Coord transpose() const { return Coord{y, x}; }
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
   int min = std::numeric_limits<int>::max();
};

static std::array<int, 10> HEAT{16, 18, 20, 56, 91, 160, 166, 178, 184, 229};
static const std::array<Coord, 4> AROUND = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};

struct Map
{
   explicit Map(std::ifstream file)
   {
      for (std::string line; std::getline(file, line);)
         map.emplace_back(line | transform([](char c) { return Tile{c}; }) | to<std::vector>);

      for (auto&& row : map)
         assert(row.size() == width());
   }

   std::vector<std::vector<Tile>> map;

   int width() const { return map[0].size(); }
   int height() const { return map.size(); }

   Tile& at(const Coord& c)
   {
      if (c.y >= 0 && c.y < map.size())
         if (c.x >= 0 && c.x <= map[c.y].size())
            return map[c.y][c.x];

      static Tile WALL = {'#'};
      return WALL; // yes I know I'm returning a non-const reference to a static here
   }

   void dump() const
   {
      for (auto& row : map)
      {
         for (auto& cell : row)
            fmt::print("\x1b[48;5;{}m{}\x1b[0m", HEAT[cell.symbol - '0'], cell.symbol);
         fmt::println("");
      }
   }

   void trace(Coord pos, Coord dir, int run)
   {
      // TODO:
   }

};

int main(int argc, char* argv[])
{
   for (size_t i = 0; i < 10; ++i)
      fmt::print("\x1b[48;5;{}m {} \x1b[0m", HEAT[i], i);
   fmt::println("");

   Map map(input(argc, argv));
   map.dump();
}
