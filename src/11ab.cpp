//
// https://adventofcode.com/2023/day/11
//
#include <set>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
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

int computeDistances(std::ifstream& file, int factor)
{
   assert(factor >= 1);

   //
   // parse input file, applying vertical expansion on the fly
   //
   std::set<Coord> galaxies;
   {
      file.clear();
      file.seekg(0);
      std::string line;
      for (int y = 0; std::getline(file, line); y++)
      {
         if (all_of(line, [](char c) { return c == '.'; }))
            y += factor - 1;
         else
            for (auto galaxy : zip(line, iota(0)) | filter([](auto p) { return p.first == '#'; }) |
                                  transform(
                                     [&](auto p) {
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
      auto used_x = galaxies | transform(&Coord::x) | to<std::set>;
      auto it = galaxies.begin();
      int expansion = 0, last_x = *used_x.begin();
      for (int x : used_x)
      {
         expansion += std::max(0, x - last_x - 1) * (factor - 1);
         while (it->x == x)
            expanded.insert(*it++ + Coord{expansion, 0});
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

   return sum;
}

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);
   fmt::println("A: {}", computeDistances(file, 2));
   fmt::println("B: {} (*10)", computeDistances(file, 10));
   fmt::println("B: {} (*100)", computeDistances(file, 100));
   fmt::println("B: {} (*1M)", computeDistances(file, 1'000'000));
}