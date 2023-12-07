#include <cassert>
#include <deque>
#include <fstream>
#include <string>

#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include <fmt/ostream.h>

constexpr auto to_number = transform([](auto s) { return std::stol(s.str()); });

struct Range
{
   long start;
   long end;

   Range operator+(long delta) const { return Range{start + delta, end + delta}; }

   auto operator<=>(const Range& other) const noexcept { return end <=> other.end; }
   // allow heterogenous lookup for end of source range, for use with lower_bound()
   auto operator<=>(long other_end) const noexcept { return end <=> other_end; }
};

struct Map
{
   std::string from;
   std::string to;

   // maps source ranges to destination ranges, with heterogenous lookup for end of source range
   std::map<Range, Range, std::less<>> ranges;
};

long delta(decltype(Map::ranges.begin())& it) { return it->second.start - it->first.start; }

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   std::string line;
   std::getline(file, line);

   boost::smatch what;
   const boost::regex seeds_regexp{R"(seeds:(?: +(\d+))+)"};
   boost::regex_match(line, what, seeds_regexp, boost::match_extra | boost::match_perl);
   auto seeds = what[1].captures() | to_number | to<std::vector>;
   fmt::println("seeds: [{}]", fmt::join(seeds, ", "));

   std::vector<Map> process;

   std::getline(file, line);
   const boost::regex category_regexp{R"((\w+)-to-(\w+) map:)"};
   const boost::regex map_entry_regexp{R"((\d+) (\d+) (\d+))"};
   while (std::getline(file, line))
   {
      while (line.empty())
         std::getline(file, line);

      boost::regex_match(line, what, category_regexp, boost::match_perl);
      auto from_category = what[1].str();
      auto to_category = what[2].str();
      fmt::println("{} to {}:", from_category, to_category);

      auto& map = process.emplace_back(what[1].str(), what[2].str());

      for (std::getline(file, line); !line.empty(); std::getline(file, line))
      {
         fmt::println("{}", line);
         boost::regex_match(line, what, map_entry_regexp, boost::match_perl);
         auto n = what | drop(1) | to_number | to<std::vector>; // dest, source, size

         map.ranges.emplace(Range{n[1], n[1] + n[2]}, Range{n[0], n[0] + n[2]});
      }
   }

   std::function<void(int, Range, long&)> recurse = [&](int step, Range seed, long& minimum)
   {
      if (seed.start >= seed.end)
         return;

      if (step == process.size())
      {
         minimum = std::min(minimum, seed.start);
         return;
      }

      //
      // seed:          [-----------[
      // ranges: [-A-[    [-C-[   [-D-[    [-E-[
      // output         [0|-1-|-2-|3[
      //

      // skip A
      auto it = process[step].ranges.lower_bound(seed.start);

      // emit B, and then as many C as are completely covered
      Range range{0, it == process[step].ranges.end() ? seed.start : it->first.end};
      for (; it != process[step].ranges.end() && it->first.start < seed.end; ++it)
      {
         range = Range{range.end, std::max(it->first.start, seed.start)};
         recurse(step + 1, range, minimum);

         range = Range{range.end, std::min(it->first.end, seed.end)};
         recurse(step + 1, range + delta(it), minimum);
      }

      // emit D
      recurse(step + 1, Range{range.end, seed.end}, minimum);
   };

   //
   // part A, which is just a subset of part B with fixed intervals of size 1
   //
   auto A = std::numeric_limits<long>::max();
   for (auto range : seeds | transform([](auto seed) -> Range { return {seed, seed + 1}; }))
      recurse(0, range, A);

   fmt::println("part A: {}", A);

   //
   // part B
   //
   long B = std::numeric_limits<long>::max();
   for (auto range : zip(seeds | stride(2), seeds | drop(1) | stride(2)) |
                        transform(
                           [](auto pair) -> Range {
                              return {pair.first, pair.first + pair.second};
                           }))
      recurse(0, range, B);

   fmt::println("part B: {}", B);
}
