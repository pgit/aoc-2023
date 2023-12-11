//
// https://adventofcode.com/2023/day/11
//
#include <set>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/view/filter.hpp>
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
   auto operator<=>(const Coord& r) const = default;
   Coord operator+(const Coord& r) const { return Coord{x + r.x, y + r.y}; }
};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   const int EXPANSION = 2; // A: 2 B: 1'000'000

   //
   // parse input file, applying vertical expansion on the fly
   //
   std::set<int> used_x;
   std::set<Coord> galaxies;
   {
      std::string line;
      for (int y = 0; std::getline(file, line); y++)
      {
         if (all_of(line, [](char c) { return c == '.'; }))
            y += EXPANSION - 1;
         else
            for (auto galaxy : zip(line, iota(0)) | filter([](auto p) { return p.first == '#'; }) |
                                  transform(
                                     [&](auto p)
                                     {
                                        used_x.insert(p.second);
                                        return Coord{p.second, y};
                                     }))
               galaxies.insert(galaxy);
      }
   }

   //
   // Apply horizontal expansion by going through used x-Coordinates from left to right.
   // This works because Coords is using the default <=> operator, which imposes a lexicographical
   // ordering on the [x, y] members.
   //
   std::set<Coord> expanded;
   {
      auto it = galaxies.begin();
      int expansion = 0, last_x = *used_x.begin();
      for (int x : used_x)
      {
         expansion += std::max(0, x - last_x - 1) * (EXPANSION - 1);
         for (; it->x == x; ++it)
            expanded.insert(*it + Coord{expansion, 0});
         last_x = x;
      }
   }

   //
   // finally, sum distances, counting each pair only once
   //
   size_t sum = 0;
   for (auto ia = expanded.begin(); ia != expanded.end(); ++ia)
      for (auto ib = ia; ib != expanded.end(); ++ib)
         sum += abs(ib->x - ia->x) + abs(ib->y - ia->y);
   fmt::println("A/B: {} (factor {})", sum, EXPANSION);
}
