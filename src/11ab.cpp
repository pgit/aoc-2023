//
// https://adventofcode.com/2023/day/11
//
#include <set>
#include <string>
using namespace std::literals;

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
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

   const int EXPANSION = 2; // 1'000'000; // A: 2 B: 1'000'000

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
   // apply horizontal expansion by going through used x-Coordinates from left to right
   //
   std::set<Coord> expanded;
   {
      int expansion = 0, last_x = *used_x.begin();
      for (int x : used_x)
      {
         expansion += std::max(0, x - last_x - 1) * (EXPANSION - 1);
         last_x = x;
         for (auto it = galaxies.lower_bound(Coord{x, std::numeric_limits<int>::min()});
              it != galaxies.upper_bound(Coord{x, std::numeric_limits<int>::max()}); ++it)
            expanded.insert(*it + Coord{expansion, 0});
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
