#include <cassert>
#include <deque>
#include <fstream>
#include <set>
#include <string>

#include <range/v3/algorithm/fold_left.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges::views;

#include <fmt/ostream.h>

constexpr bool is_symbol(char c) { return c != '.' && !isdigit(c); }

const char* getn(const char* p)
{
   if (!isdigit(*p))
      return nullptr;
   while (isdigit(*--p))
      ;
   return p + 1;
}

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   std::deque<std::string> plan;
   for (std::string line; std::getline(file, line);)
      plan.emplace_back(fmt::format(".{}.", line));
   plan.emplace_front(plan[0].size(), '.');
   plan.emplace_back(plan[0].size(), '.');

   size_t sum = 0, gears = 0;
   for (auto [la, lb, lc] : zip(plan, plan | drop(1), plan | drop(2)))
   {
      std::optional<size_t> number = 0;
      bool has_symbol = false;
      for (auto [a, b, c] : zip(la, lb, lc))
      {
         bool symbol = is_symbol(a) || is_symbol(b) || is_symbol(c);
         has_symbol |= symbol;
         if (std::isdigit(b)) // start of number
            number = (number ? *number * 10 : 0) + b - '0';
         else if (number) // after end of number
         {
            if (has_symbol)
               sum += *number;
            number.reset();
            has_symbol = symbol;
         }
         else // no number
            has_symbol = symbol;
      }
   }

   //
   // part b
   //
   const size_t w = plan[0].size(), h = plan.size();
   auto map = new char[w * h];
   for (auto [y, l] : zip(iota(0), plan))
      for (auto [x, c] : zip(iota(0), l))
         map[y * w + x] = c;

   const std::array<std::pair<int, int>, 8> offsets{{
      // clang-format off
      {-1, -1}, { 0, -1}, { 1, -1}, //
      {-1,  0}, /*    */  { 1,  0}, //
      {-1,  1}, { 0,  1}, { 1,  1}, //
      // clang-format on
   }};

   for (int y = 1; y < h - 1; ++y)
   {
      for (int x = 1; x < w - 1; ++x)
      {
         if (map[y * w + x] == '*')
         {
            std::set<const char*> pointers;
            for (auto [dx, dy] : offsets)
               if (auto* pn = getn(map + (y + dy) * w + x + dx))
                  pointers.insert(pn);

            if (pointers.size() == 2)
               gears += atoi(*pointers.begin()) * atoi(*pointers.rbegin());
         }
      }
   }

   fmt::println("sum: {} gears={}", sum, gears);
}
