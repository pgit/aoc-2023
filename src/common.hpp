#include <filesystem>
namespace fs = std::filesystem;

#include <fmt/ostream.h>

inline std::ifstream input(int argc, char* argv[])
{
   if (argc > 1)
      return std::ifstream(argv[1]);

   auto day = std::stoi(fs::path(argv[0]).filename().string());
   auto filename = fmt::format("input/{}.txt", day);
   return std::ifstream(filename);
}
