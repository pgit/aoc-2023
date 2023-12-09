#include <string>

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include "common.hpp"

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   size_t power = 0;
   size_t sum = 0;
   const boost::regex regexp{R"(Game ([0-9]+):(?: ([0-9]+ [a-z]+)[;,]?)+)"};
   for (std::string line; std::getline(file, line);)
   {
      boost::smatch what;
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);

      fmt::println("LINE: {}", line);

      bool invalid = false;
      std::map<std::string, size_t> hist;
      size_t index = stol(what[1]);
      for (auto& capture : what[2].captures())
      {
         auto cap = std::string_view(capture.begin(), capture.end());
         size_t n = atoi(cap.data());
         cap = cap.substr(cap.find_first_of(' ') + 1);
         invalid |= cap == "red" && n > 12 || cap == "green" && n > 13 || cap == "blue" && n > 14;
         hist[std::string(cap)] = std::max(hist[std::string(cap)], n);
      }

      if (!invalid)
         sum += index;

      power += hist["red"] * hist["green"] * hist["blue"];
   }

   fmt::println("The result is: {} power={}", sum, power);
}
