#include <cassert>
#include <charconv>
#include <fstream>
#include <string>
using namespace std::literals;

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include <fmt/ostream.h>

std::optional<std::string> getline(std::ifstream& file)
{
   std::string line;
   if (std::getline(file, line))
      return line;
   else
      return std::nullopt;
}

template <class T>
const auto to_number = transform(
   [](auto&& rng)
   {
      T l{};
      if (ranges::distance(rng))
         std::from_chars(&*rng.begin(), &*rng.begin() + ranges::distance(rng), l);
      return l;
   });

std::pair<long, long> delta(const std::vector<long>& numbers)
{
   fmt::println("[{}]", fmt::join(numbers, " "));

   if (all_of(numbers, [&](auto n) { return n == 0; }))
      return std::make_pair(0, 0);

   std::vector deltas = zip(numbers, numbers | drop(1)) |
                        transform([](auto pair) { return pair.second - pair.first; }) |
                        to<std::vector>;

   auto decend = delta(deltas);
   return {decend.first + numbers[numbers.size() - 1], numbers[0] - decend.second};
}

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   long A = 0, B = 0;
   while (auto line = getline(file))
   {
      auto numbers = *line | split(' ') | to_number<long> | to<std::vector>;
      auto deltas = delta(numbers);
      A += deltas.first;
      B += deltas.second;
      fmt::println("");
   }
   
   fmt::println("A: {} B: {}", A, B);
}
