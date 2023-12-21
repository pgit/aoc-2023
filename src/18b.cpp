//
// https://adventofcode.com/2023/day/18
//
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
   Coord operator-() const { return Coord{-x, -y}; }
   Coord transpose() const { return Coord{y, x}; }
   Coord operator+(const Coord& r) const { return Coord{x + r.x, y + r.y}; }
   Coord operator-(const Coord& r) const { return Coord{x - r.x, y - r.y}; }
   Coord operator*(int f) const { return Coord{x * f, y * f}; }
   const Coord& operator+=(const Coord& r)
   {
      x += r.x;
      y += r.y;
      return *this;
   }
   Coord min(const Coord& r) { return {std::min(x, r.x), std::min(y, r.y)}; }
   Coord max(const Coord& r) { return {std::max(x, r.x), std::max(y, r.y)}; }
   bool operator==(const Coord& r) const noexcept = default;
};

const std::map<char, Coord> DIRECTIONS = {
   {'0', Coord{1, 0}},
   {'1', Coord{0, 1}},
   {'2', Coord{-1, 0}},
   {'3', Coord{0, -1}},
};


using Interval = boost::icl::interval<int>::type;

template <>
struct fmt::formatter<Interval>
{
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx)
   {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(Interval const& interval, FormatContext& ctx)
   {
      interval.bounds().left();
      return fmt::format_to(ctx.out(), "{2}{0},{1}{3}", interval.lower(), interval.upper(),
                            interval.bounds().bits() & 1 ? '[' : ']',
                            interval.bounds().bits() & 2 ? ']' : '[');
   }
};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   Coord pos;

   //
   // For part B, record map of horizontal lines.
   //
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
   boost::icl::interval_set<int> active, last;
   int y0 = lines.begin()->first;
   for (auto [y, interval] : lines)
   {
      //
      //  0   ┌─────┐ 
      //  1   │.....│ 
      //  2   └─┐...│ ----    Stop at each distinct 'y' that has a different set of active      
      //  3   ..│...│         horizontal lines. Compute size of rectangles covered by delta-y
      //  4   ..│...│         and active horizontal lines.
      //  5   ┌─┘.┌─┘ ----
      //  6   │...│..
      //  7   └┐..└─┐ ----
      //  8   .│....│
      //  9   .└────┘
      // 
      if (y != y0)
      {
         for (auto i : active)
            B += (y - y0) * (i.upper() - i.lower() + 1);

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
         //      .└────┘
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
