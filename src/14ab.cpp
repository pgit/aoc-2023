//
// https://adventofcode.com/2023/day/13
//
#include <set>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include "range/v3/view/subrange.hpp"
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

struct Coord
{
   int x, y;
};

enum class Direction : int
{
   west = 0,
   north,
   east,
   south
};

struct Map
{
   Map(size_t w_, size_t h_) : w(w_), h(h_), dy(w + 2), m_data((w + 2) * (h + 2), 0) {}

   struct ColumnIterator
   {
      char* p;
      int d;

      char& operator*() const { return *p; }
      ColumnIterator& operator++()
      {
         assert(*p);
         p += d;
         return *this;
      }
      const ColumnIterator operator++(int)
      {
         ColumnIterator result(*this);
         assert(*p);
         p += d;
         return result;
      }
      bool operator==(const ColumnIterator& other) const { return p == other.p; }
      bool operator==(nullptr_t) const { return *p == 0; }
      ssize_t operator-(const ColumnIterator& other) const { return ssize_t(p - other.p) / d; }
   };

   struct Row
   {
      char* p0;
      const int dcol;

      ColumnIterator begin() const { return {p0, dcol}; }
      nullptr_t end() const { return nullptr; }
   };

   struct RowIterator
   {
      Row row;
      const int drow;

      Row& operator*() { return row; }
      RowIterator operator++()
      {
         row.p0 += drow;
         return *this;
      }
      bool operator==(const RowIterator& other) const { return row.p0 == other.row.p0; }
      bool operator==(nullptr_t) const { return *row.p0 == 0; }
   };

   struct View
   {
      Map& map;
      char* p0;
      int drow, dcol; // delta within view to go to next row / column
      Row row(int r) const { return Row{p0 + r * drow, dcol}; }

      RowIterator begin() const { return {row(0), drow}; }
      nullptr_t end() const { return nullptr; }
   };

   inline char* data() { return m_data.data() + dy + 1; }
   inline int delta(int x, int y) const { return y * dy + x; }
   inline char* pos(int x, int y) { return data() + delta(x, y); }
   inline char& at(int x, int y) { return *pos(x, y); }

   View view(Direction dir)
   {
      switch (dir)
      {
         // clang-format off
      case Direction::west:
         return View{*this, pos(  0,   0), delta( 0,  1), delta( 1,  0)};
      case Direction::north:
         return View{*this, pos(w-1,   0), delta(-1,  0), delta( 0,  1)};
      case Direction::east:
         return View{*this, pos(w-1, h-1), delta( 0, -1), delta(-1,  0)};
      case Direction::south:
         return View{*this, pos(  0, h-1), delta( 1,  0), delta( 0, -1)};
         // clang-format on
      }
   }

   size_t hash() const
   {
      return std::hash<std::string_view>{}(std::string_view(m_data.data(), m_data.size()));
   }

   const int w, h, dy;
   std::vector<char> m_data;
};

// reversed weight function, pass south view if you want the north weight
size_t weight(Map::Row& row)
{
#if 1
   size_t result = 0, weight = 0;
   for (auto c : row)
   {
      ++weight;
      if (c == 'O')
         result += weight;
   }
   return result;
#else
   return 0;
#endif
}

void slide(Map::Row& vec)
{
   for (auto p0 = vec.begin(); p0 != vec.end();)
   {
      //
      //    OO.O.O..#.O
      // p0 ^
      //
      // fmt::println("p0={} *p0='{}'", p0 - vec.begin(), *p0);
      while (*p0 != 0 && *p0 != '.')
         ++p0;

      //
      //    OO.O.O..#.O
      // p0   ^
      // p1
      auto p1 = p0;
      // fmt::println("p0={} p1={} *p0='{}' *p1='{}'", p0 - vec.begin(), p1 - vec.begin(), *p0,
      // *p1);
      while (*p1 == '.')
         ++p1;

      //
      //    OO.O.O..#.O
      // p0   ^
      // p1    ^
      //
      // fmt::println("p0={} p1={} *p0='{}' *p1='{}'", p0 - vec.begin(), p1 - vec.begin(), *p0,
      // *p1);
      while (*p1 == 'O')
         *p0++ = 'O', *p1++ = '.';

      if (*p1 != '.')
         p0 = p1;

      //
      //    OOO..O..#.O
      // p0    ^
      // p1     ^
      //
   }
}

void slide(Map& map, Direction dir)
{
   for (auto row : map.view(dir))
      slide(row);
}

size_t weight(Map& map, Direction dir)
{
   size_t total = 0;
   for (auto row : map.view(dir))
      total += weight(row);
   return total;
}

void dump(Map& map)
{
   for (auto row : map.view(Direction::west))
   {
      for (auto c : row)
         fmt::print("{}", c);
      fmt::println("");
   }
   fmt::println("");
}

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   std::vector<std::string> rows;
   for (std::string row; std::getline(file, row) && !row.empty();)
      rows.emplace_back(row);

   const auto w = rows[0].size(), h = rows.size();
   Map map(w, h);

   std::vector<std::string> columns;
   for (size_t i = 0; i < rows[0].size(); ++i)
   {
      assert(rows[i].size() == w);
      memcpy(map.pos(0, i), rows[i].data(), w);
      columns.emplace_back(rows | transform([&](auto& str) { return str[i]; }) | to<std::string>);
   }

   //
   // part A: slide north once
   //
   slide(map, Direction::north);

   dump(map);

   size_t A = weight(map, Direction::south);
   fmt::println("A: {}", A);

   //
   // part B: perforam 1M cycles -- which takes forever, but there are loops
   //
   std::size_t t0 = 0, h0 = 0, loop_size = 0;
   std::set<size_t> hashes;
   const size_t CYCLES = 1'000'000'000;
   for (size_t cycle = 0; cycle < CYCLES; ++cycle)
   {
      // fmt::println("Cycle {}", cycle);

      //
      // perform a cycle of sliding N, W, S, E
      //
      for (auto dir : {Direction::north, Direction::west, Direction::south, Direction::east})
         for (auto row : map.view(dir))
            slide(row);

      //
      // detect loops by keeping a set of hashes
      //
      auto hash = map.hash();
      if (!hashes.insert(hash).second && !t0)
      {
         t0 = cycle, h0 = hash;
         fmt::println("start of loop: {}", t0);
      }
      else if (h0 == hash && !loop_size)
      {
         loop_size = cycle - t0;
         fmt::println("end of loop at {}, size {}", cycle, cycle - t0);
         size_t forward_loops = (CYCLES - cycle) / loop_size;
         cycle += forward_loops * loop_size;
         fmt::println("forwarded {} loops to {}", forward_loops, cycle);
      }
      else if (h0 == hash)
         assert((cycle - t0) % cycle_size == 0);
   }

   size_t B = weight(map, Direction::south);
   fmt::println("B: {}", B);
}
