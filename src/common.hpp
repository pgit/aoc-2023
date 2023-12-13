#include <filesystem>
#include <optional>
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

inline std::optional<std::string> getline(std::ifstream& file)
{
   std::string line;
   if (std::getline(file, line))
      return line;
   else
      return std::nullopt;
}

namespace fmt
{
template <typename T>
struct formatter<std::optional<T>> : fmt::formatter<T>
{

   template <typename FormatContext>
   auto format(const std::optional<T>& opt, FormatContext& ctx)
   {
      if (opt)
      {
         fmt::formatter<T>::format(*opt, ctx);
         return ctx.out();
      }
      return fmt::format_to(ctx.out(), "NO VALUE");
   }
};
} // namespace fmt