#include <cassert>
#include <fstream>
#include <string>

#include <fmt/ostream.h>

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

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
