#include <cassert>
#include <cmath>
#include <string>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include "common.hpp"

constexpr auto to_number = transform([](auto s) { return std::stol(s.str()); });

auto get_numbers(auto& file)
{
   std::string line;
   std::getline(file, line);
   boost::smatch what;
   static const boost::regex regexp{R"(\w+:(?: +(\d+))+)"};
   boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
   return what[1].captures() | to_number | to<std::vector>;
};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   auto times = get_numbers(file);
   auto distances = get_numbers(file);

   fmt::println("times: [{}]", fmt::join(times, ", "));
   fmt::println("distances: [{}]", fmt::join(distances, ", "));

   //
   //    (t - s) * s    < d  <=>
   //      s^2 + t*s -d < 0
   //                 s < -(t/2) + sqrt((t/2)^2 + d) < d
   //

   auto count = [](long time, long distance) {
      long victories = 0;
      long speed = 0;
      while (speed < time && (time - speed) * speed <= distance)
         ++speed;

      while (speed < time && (time - speed) * speed > distance)
         ++speed, ++victories;

      double t2 = double(time) / 2;
      auto calc = std::ceil(std::sqrt((t2 * t2) - distance) + t2) -
                  std::ceil(-std::sqrt((t2 * t2) - distance) + t2);

      assert(victories == calc);
      return victories;
   };

   //
   // part A
   //
   long A = 0;
   for (auto [time, distance] : zip(times, distances))
   {
      auto victories = count(time, distance);
      A = A ? A * victories : victories;
   }
   fmt::println("A={}", A);

   //
   // part B
   //
   auto time = std::stol(fmt::format("{}", fmt::join(times, "")));
   auto distance = std::stol(fmt::format("{}", fmt::join(distances, "")));
   auto B = count(time, distance);
   fmt::println("B={}", B);
}
