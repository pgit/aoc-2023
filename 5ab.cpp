#include <cassert>
#include <deque>
#include <fstream>
#include <string>

#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
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
   long dest;
   long size;
};

struct Map
{
   std::string from;
   std::string to;
   
   // maps a+x to [b,x] for [a,a+x] -> [b,b+x] (lookup by end of source range)
   std::map<long, Range> ranges; 
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
         auto n = what | drop(1) | to_number | to<std::vector>;
         map.ranges.emplace(n[1] + n[2] /* end of source range */, Range(n[0], n[2]));
      }
   }

   auto maxSeed = std::numeric_limits<long>::max();
   for (auto seed : seeds)
   {
      fmt::print("seed {}:", seed);
      for (auto& map : maps)
      {
         auto it = map.ranges.lower_bound(seed);
         if (it != map.ranges.end() && seed > it->first - it->second.size)
         {
            auto delta = it->second.dest - it->first + it->second.size;
            seed += delta;
            fmt::print(" [{}]", delta);
         }
         else
            fmt::print(" []");
         fmt::print(" {}", seed);
      }
      fmt::println("");

      maxSeed = std::min(maxSeed, seed);
   }

   fmt::println("minSeed: {}", maxSeed);
}
