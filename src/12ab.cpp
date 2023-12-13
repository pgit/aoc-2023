//
// https://adventofcode.com/2023/day/12
//
#include <set>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/repeat_n.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include "common.hpp"

constexpr auto to_number = transform([](auto s) { return std::stoi(s.str()); });

size_t countVariants(std::string_view pattern, std::optional<int> n, std::span<int> groups)
{
   // fmt::println("{:8} at {} [{}]", n, pattern, fmt::join(groups, ","));

   if (pattern.empty())
      return (!n && groups.empty()) ? 1 : 0;

   switch (pattern[0])
   {
   default:
      assert(false);

   case '.':
      if (!n || n == 0) // skip '.' if not within a group
         return countVariants(pattern.substr(1), std::nullopt, groups);
      else // '.' while still within a group: invalid
         return 0;

   case '#':
      if (!n) // start a new group
         return groups.empty() ? 0 : countVariants(pattern, groups[0], groups.subspan(1));
      else if (n == 0) // group finished, but still a '#' left to fill
         return 0;
      else // continue with group
         return countVariants(pattern.substr(1), *n - 1, groups);

   case '?':
      size_t count = 0;
      if (!n)
         return
            // treat '?' as '.' and skip it
            countVariants(pattern.substr(1), std::nullopt, groups) +
            // treat '?' as '#' and start a group
            (groups.empty() ? 0 : countVariants(pattern, groups[0], groups.subspan(1)));
      else if (n == 0) // treat as '.' and finish group
         return countVariants(pattern.substr(1), std::nullopt, groups);
      else // treat as '#' and continue with group
         return countVariants(pattern.substr(1), *n - 1, groups);
   }
}

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   size_t A = 0, B = 0;
   const boost::regex regexp{R"(([.#?]+) (?:(\d+),?)*)"};
   for (std::string line; std::getline(file, line) && !line.empty();)
   {
      boost::smatch what;
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
   
      auto pattern = what[1].str();
      auto groups = what[2].captures() | to_number | to<std::vector>;
      auto count = countVariants(pattern + '.', std::nullopt, groups);
      fmt::println("{} {} -> {}", pattern, fmt::join(groups, ","), count);
      A += count;
      
      pattern = repeat_n(what[1].str(), 5) | join('?') | to<std::string>;
      groups = repeat_n(what[2].captures() | to_number, 5) | join | to<std::vector>;
      fmt::println("{} {}", pattern, fmt::join(groups, ","));
      count = countVariants(pattern + '.', std::nullopt, groups);
      fmt::println("{} {} -> {}", pattern, fmt::join(groups, ","), count);
      B += count;
   }

   fmt::println("A: {}", A);
   fmt::println("B: {}", B);
}