//
// https://adventofcode.com/2023/day/18
//
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
using namespace ranges;
using namespace ranges::views;

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include <fmt/format.h>

#include "common.hpp"

struct Coord
{
   long x = 0, y = 0;
   Coord operator-(const Coord& r) const { return Coord{x - r.x, y - r.y}; }
   Coord operator*(int f) const { return Coord{x * f, y * f}; }
   const Coord& operator+=(const Coord& r)
   {
      x += r.x;
      y += r.y;
      return *this;
   }
};

const std::map<char, Coord> DIRECTIONS = {
   {'0', Coord{1, 0}},
   {'1', Coord{0, 1}},
   {'2', Coord{-1, 0}},
   {'3', Coord{0, -1}},
};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   Coord pos;

   //
   // For part B, record map of horizontal lines.
   //
   using Interval = boost::icl::interval<long>::type;
   std::multimap<long, Interval> lines;
   const boost::regex regexp{R"(([LRUD]) ([0-9]+) \(#([a-zA-Z0-9]{5})([a-zA-Z0-9])\))"};
   for (std::string line; std::getline(file, line);)
   {
      boost::smatch what;
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
      auto dir = DIRECTIONS.at(what[4].str()[0]);
      auto steps = std::stol(what[3].str(), nullptr, 16);
      pos += dir * steps;
      if (dir.x)
      {
         auto p0 = pos - dir * steps;
         auto p1 = pos;
         lines.emplace(pos.y, Interval::closed(std::min(p0.x, p1.x), std::max(p0.x, p1.x)));
      }
   }

   //
   // part B (for map of part A)
   //
   size_t B = 0;
   boost::icl::interval_set<long> active, last;
   int y0 = lines.begin()->first;
   for (auto [y, interval] : lines)
   {
      //      0     6
      //  0   ┌─────┐
      //  1   │.....│ 2x7
      //  2   └─┐...│ ----    Stop at each distinct 'y' that has a different set of active
      //  3   ..│...│         horizontal lines. Compute size of rectangles covered by delta-y
      //  4   ..│...│ 3x5     and active horizontal lines.
      //  5   ┌─┘.┌─┘ ----
      //  6   │...│.. 2x5
      //  7   └┐..└─┐ ----
      //  8   .│....│ 3x6     Intermediate result: 57
      //  9   .└────┘
      //
      if (y != y0)
      {
         B += accumulate(active | transform([&](auto i) { return (y - y0) * length(i); }), 0L);

         //
         //      ┌─────┐
         //      │.....│
         // 2 >> └─┐...│    Each time we remove something from the 'active' intervals,
         //      ..│...│    we have to count those blocks extra that have been removed.
         //      ..│...│
         //      ┌─┘.┌─┘ << 2
         //      │...│..
         // 1 >> └┐..└─┐
         //      .│....│
         //      .└────┘   Final result: 57 + 5 = 62
         //
         B += length(last - active);
         last = active;
         y0 = y;
      }

      if (!boost::icl::contains(active, interval))
         active.add(interval);
      else
      {
         active.subtract(interval);

         //
         // By subtracting, we may have broken some intervals.
         //
         //   A  B  C
         //   [-----] active interval
         //      [--] subtract closed interval
         //   [--[    result

         //   [--]    <-- add B -- closes interval but does not change size
         //   [--[  I <-- add C -- adds another zero-sized interval
         //
         auto checkBorder = [&](int pos)
         {
            const auto n0 = active.iterative_size();
            active.add(pos);
            if (n0 != active.iterative_size())
               active.subtract(pos); // this was an I
         };
         checkBorder(interval.lower());
         checkBorder(interval.upper());
      }
   }

   B += length(last);

   fmt::println("B: {}", B);
}
