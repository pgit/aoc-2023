//
// https://adventofcode.com/2023/day/9
//
#include <sstream>
#include <string>
using namespace std::literals;

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include "range/v3/view/istream.hpp"
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

struct Result
{
   long front = {};
   long back = {};
   const Result& operator+=(const Result& other) noexcept
   {
      front += other.front;
      back += other.back;
      return *this;
   }
};

Result delta(const std::vector<long>& numbers)
{
   fmt::println("[{}]", fmt::join(numbers, " "));

   if (all_of(numbers, [&](auto n) { return n == 0; }))
      return {};

   std::vector deltas = zip(numbers, numbers | drop(1)) |
                        transform([](auto pair) { return pair.second - pair.first; }) |
                        to<std::vector>;

   auto decend = delta(deltas);
   auto result = Result{numbers.front() - decend.front, decend.back + numbers.back()};
   fmt::println("[{} {} {}]", result.front, fmt::join(numbers, " "), result.back);
   return result;
}

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   Result result;
   std::string line;
   while (std::getline(file, line))
   {
      std::istringstream ss{line};
      auto numbers = istream<long>(ss) | ranges::to<std::vector>;
      result += delta(numbers);
      fmt::println("");
   }

   fmt::println("A: {} B: {}", result.back, result.front);
}
