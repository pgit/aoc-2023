#include <cassert>
#include <string>
#include <vector>

#include <range/v3/view.hpp>
namespace rv = ranges::views;

#include "common.hpp"

static const std::vector<std::string> digits = {"one", "two",   "three", "four", "five",
                                                "six", "seven", "eight", "nine"};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   size_t sum = 0;
   std::string line;
   while (std::getline(file, line))
   {
      auto sv = std::string_view(line);
      if (sv.empty())
         continue;
      auto n0 = sv.find_first_of("1234567890");
      auto n1 = sv.find_last_of("1234567890");

      const auto npos = std::string::npos;
      auto [s0, v0] = std::invoke(
         [&]() -> std::pair<size_t, int>
         {
            for (size_t i = 0; i < sv.size(); ++i)
               for (auto [digit, n] : rv::zip(digits, rv::iota(1)))
                  if (sv.substr(i).starts_with(digit))
                     return {i, n};
            return {npos, 0};
         });

      auto [s1, v1] = std::invoke(
         [&]() -> std::pair<size_t, int>
         {
            for (size_t i = sv.size() - 1; i != npos; --i)
               for (auto [digit, n] : rv::zip(digits, rv::iota(1)))
                  if (sv.substr(i).starts_with(digit))
                     return {i, n};
            return {npos, 0};
         });

      assert(n0 != npos || s0 != npos);
      assert(n1 != npos || s1 != npos);

      auto d0 = ((s0 == npos || n1 != npos && n0 < s0) ? (sv[n0] - '0') : v0);
      auto d1 = ((s1 == npos || n1 != npos && n1 > s1) ? (sv[n1] - '0') : v1);
      auto num = 10 * d0 + d1;
      fmt::println("{} {}", sv, num);
      sum += num;
   }

   fmt::println("The result is: {}", sum);
}
