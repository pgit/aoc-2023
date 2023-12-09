#include <string>

#include "common.hpp"

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   size_t sum = 0;
   std::string line;
   while (std::getline(file, line))
   {
      auto pos = line.find_first_of("1234567890");
      if (pos == std::string::npos)
         continue;

      sum += (line[pos] - '0') * 10;
      sum += line[line.find_last_of("1234567890")] - '0';
   }

   fmt::println("The result is: {}", sum);
}
