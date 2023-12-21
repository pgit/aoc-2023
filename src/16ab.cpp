//
// https://adventofcode.com/2023/day/16
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
   bool visited = false;
};

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
            if (cell.visited)
               fmt::print("\x1b[44m{}\x1b[0m", cell.symbol);
            else
               fmt::print("{}", cell.symbol);
         fmt::println("");
      }
   }

   void trace(Coord pos, Coord dir)
   {
      for (;;)
      {
         pos += dir;

         auto& tile = at(pos);
         if (tile.symbol == '#')
            return;

         switch (tile.symbol)
         {
         case '#':
            return; // wall, stop tracing

         case '/':
            dir = -dir.transpose(); // reflection
            break;

         case '\\':
            dir = dir.transpose(); // reflection
            break;

         case '-':
            if (dir.x)
               break; // horizontal pass through

            if (tile.visited)
               return; // endless loop otherwise

            tile.visited = true;
            trace(pos, Coord{-1, 0});
            trace(pos, Coord{1, 0});
            return;

         case '|':
            if (dir.y)
               break; // vertical pass through

            if (tile.visited)
               return; // endless loop otherwise

            tile.visited = true;
            trace(pos, Coord{0, -1});
            trace(pos, Coord{0, 1});
            return;
         }

         tile.visited = true;
      }
   }

   // count visited tiles and reset visited state 
   int takeVisted()
   {
      int result = 0;
      for (auto& row : map)
         for (auto& cell : row)
            if (cell.visited)
            {
               cell.visited = false;
               result++;
            }
      return result;
   }
};

int main(int argc, char* argv[])
{
   Map map(input(argc, argv));

   Coord pos{-1, 0}, dir{1, 0};
   map.trace(pos, dir);
   map.dump();
   size_t A = map.takeVisted();

   int B = 0;
   for (int x = 0; x < map.width(); ++x)
   {
      map.trace({x, -1}, {0, 1});
      B = std::max(B, map.takeVisted());

      map.trace({x, map.height()}, {0, -1});
      B = std::max(B, map.takeVisted());
   }

   for (int y = 0; y < map.width(); ++y)
   {
      map.trace({-1, y}, {1, 0});
      B = std::max(B, map.takeVisted());

      map.trace({map.width(), y}, {-1, 0});
      B = std::max(B, map.takeVisted());
   }

   fmt::println("A: {}", A);
   fmt::println("B: {}", B);
}
