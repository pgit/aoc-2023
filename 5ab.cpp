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

   long size() const
   {
      assert(end >= start);
      return end - start;
   }

   Range shift(long delta) const { return Range{start + delta, end + delta}; }

   auto operator<=>(const Range& other) const noexcept { return end <=> other.end; }
   auto operator<=>(long other_end) const noexcept { return end <=> other_end; }
};

struct Map
{
   std::string from;
   std::string to;

   // maps a+x to [b,x] for [a,a+x] -> [b,b+x] (lookup by end of source range)
   std::map<Range, Range, std::less<>> ranges;
};

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

   std::vector<Map> maps;

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

      auto& map = maps.emplace_back(what[1].str(), what[2].str());

      for (std::getline(file, line); !line.empty(); std::getline(file, line))
      {
         fmt::println("{}", line);
         boost::regex_match(line, what, map_entry_regexp, boost::match_perl);
         auto n = what | drop(1) | to_number | to<std::vector>; // dest, source, size

         map.ranges.emplace(Range{n[1], n[1] + n[2]}, Range{n[0], n[0] + n[2]});
      }
   }

   auto minSeed = std::numeric_limits<long>::max();
   std::function<void(int, Range)> recurse = [&](int level, Range seed_range)
   {
      if (seed_range.start >= seed_range.end)
         return;

      if (level == maps.size())
      {
         minSeed = std::min(minSeed, seed_range.start);
         return;
      }

      auto it = maps[level].ranges.begin();
      while (it != maps[level].ranges.end() && it->first.end < seed_range.start)
         ++it;

      recurse(level + 1, Range{seed_range.start, std::min(it->first.start, seed_range.end)});

      long last_end = seed_range.start;
      if (it != maps[level].ranges.end())
         last_end = it->first.end;

      for (; it != maps[level].ranges.end() && it->first.start < seed_range.end; ++it)
      {
         auto range = it->first;
         long delta = it->second.start - it->first.start;

         recurse(level + 1, Range{last_end, std::max(range.start, seed_range.start)});
         last_end = range.end;

         recurse(level + 1,
                 Range{std::max(range.start, seed_range.start), std::min(range.end, seed_range.end)}
                    .shift(delta));
      }

      recurse(level + 1, Range{last_end, seed_range.end});
   };

   //
   // part A
   //
   for (auto range : seeds | transform([](auto seed) -> Range { return {seed, seed+1}; }))
      recurse(0, range);

   fmt::println("part A: {}", minSeed);

   //
   // part B
   //
   minSeed = std::numeric_limits<long>::max();
   for (auto range : zip(seeds | stride(2), seeds | drop(1) | stride(2)) |
                        transform(
                           [](auto pair) -> Range {
                              return {pair.first, pair.first + pair.second};
                           }))
      recurse(0, range);

   fmt::println("part B: {}", minSeed);
}
