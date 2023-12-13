//
// https://adventofcode.com/2023/day/13
//
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

int findReflection(const std::vector<std::string>& vec, int smudges)
{
   for (int pos = 1; pos < vec.size(); ++pos)
   {
      int a = pos - 1, b = pos, smudge = smudges;
      while (a >= 0 && b < vec.size() &&
             ranges::all_of(zip(vec[a], vec[b]),
                            [&](const auto& p) { return p.first == p.second || smudge-- > 0; }))
         --a, ++b;

      // this is a match only if EXACTLY the required numbers of smudges have been used
      if ((a == -1 || b == vec.size()) && smudge == 0)
         return pos;
   }

   return 0;
}

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   size_t A = 0, B = 0;
   while (!file.eof())
   {
      std::vector<std::string> rows;
      for (std::string row; std::getline(file, row) && !row.empty();)
         rows.emplace_back(row);

      std::vector<std::string> columns;
      for (size_t i : iota(0UL, rows[0].size()))
         columns.emplace_back(rows | transform([&](auto& str) { return str[i]; }) |
                              to<std::string>);

      A += findReflection(columns, 0) + 100 * findReflection(rows, 0);
      B += findReflection(columns, 1) + 100 * findReflection(rows, 1);
   }

   fmt::println("A: {}", A);
   fmt::println("B: {}", B);
}
