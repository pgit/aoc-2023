//
// https://adventofcode.com/2023/day/13
//
#include <set>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/for_each.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include "range/v3/view/filter.hpp"
#include "range/v3/view/reverse.hpp"
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

// -------------------------------------------------------------------------------------------------

struct ColumnIterator
{
   char* p = nullptr;
   ssize_t d = 0;

   using value_type = char;
   using difference_type = std::ptrdiff_t;

   char& operator*() const { return *p; }
   ColumnIterator& operator++()
   {
      p += d;
      return *this;
   }

   ColumnIterator& operator--()
   {
      p -= d;
      return *this;
   }

   ColumnIterator operator++(int) // NOLINT(cert-dcl21-cpp)
   {
      ColumnIterator temp = *this;
      p += d;
      return temp;
   }

   ColumnIterator operator--(int) // NOLINT(cert-dcl21-cpp)
   {
      ColumnIterator temp = *this;
      p -= d;
      return temp;
   }

   bool operator==(const ColumnIterator& other) const { return p == other.p; }
   difference_type operator-(const ColumnIterator& other) const { return (p - other.p) / d; }
};

static_assert(std::forward_iterator<ColumnIterator>);
static_assert(std::bidirectional_iterator<ColumnIterator>);

// -------------------------------------------------------------------------------------------------

struct Row
{
   char* p0;
   ssize_t dx = 0; // offset to next column
   size_t w;

   size_t size() const { return w; }
   auto begin() const { return ColumnIterator{p0, dx}; }
   auto end() const { return ColumnIterator{p0 + dx * w, dx}; }

   // reversed weight function, pass south view if you want the north weight
   size_t weight()
   {
      return accumulate(zip(reverse(*this), iota(1)) |
                           filter([](auto a) { return a.first == 'O'; }) |
                           transform([](auto a) { return a.second; }),
                        size_t{0});
   }

   void slide();
};

static_assert(std::ranges::range<Row>);
static_assert(ranges::range<Row>);
static_assert(ranges::input_range<Row>);
static_assert(ranges::forward_range<Row>);
static_assert(ranges::common_range<Row>); // if begin() and end() return the same type

template <>
inline constexpr bool ranges::enable_borrowed_range<Row> = true;
static_assert(ranges::borrowed_range<Row>);
static_assert(ranges::viewable_range<Row>);

// -------------------------------------------------------------------------------------------------

struct RowIterator
{
   Row row;
   ssize_t dy = 0; // offset to next row

   using value_type = Row;
   using difference_type = std::ptrdiff_t;

   Row operator*() const { return row; }
   RowIterator& operator++()
   {
      row.p0 += dy;
      return *this;
   }
   RowIterator operator++(int) // NOLINT(cert-dcl21-cpp)
   {
      RowIterator temp = *this;
      row.p0 += dy;
      return temp;
   }

   bool operator==(const RowIterator& other) const { return row.p0 == other.row.p0; }
};

static_assert(std::forward_iterator<RowIterator>);
static_assert(!std::bidirectional_iterator<RowIterator>);

struct View
{
   char* p0;
   const ssize_t dy, dx; // delta within view to go to next row / column
   const size_t w, h;

   Row row(int r) const { return Row{p0 + r * dy, dx, w}; }
   RowIterator begin() const { return {row(0), dy}; }
   RowIterator end() const { return {row(h), dy}; }
};

static_assert(std::ranges::range<View>);
static_assert(ranges::range<View>);
static_assert(ranges::input_range<View>);
static_assert(ranges::forward_range<View>);
static_assert(ranges::common_range<View>);

template <>
inline constexpr bool ranges::enable_borrowed_range<View> = true;
static_assert(ranges::borrowed_range<View>);
static_assert(ranges::viewable_range<View>);

// -------------------------------------------------------------------------------------------------

enum class Direction : int
{
   west = 0,
   north,
   east,
   south
};

struct Map
{
#if 1
   Map(size_t w_, size_t h_) : w(w_), h(h_), dy(w + 2), m_data((w + 2) * (h + 2), 0) {}
   inline char* data() { return m_data.data() + dy; }
#else
   Map(size_t w_, size_t h_) : w(w_), h(h_), dy(w), m_data(w * h, 0) {}
   inline char* data() { return m_data.data() + dy; }
#endif
   inline int delta(int x, int y) const { return x + y * dy; }
   inline char* pos(int x, int y) { return data() + delta(x, y); }
   inline char& at(int x, int y) { return *pos(x, y); }

   View view(Direction dir)
   {
      switch (dir)
      {
         // clang-format off
      case Direction::west:
         return View{pos(  0,   0), delta( 0,  1), delta( 1,  0), w, h};
      case Direction::north:
         return View{pos(w-1,   0), delta(-1,  0), delta( 0,  1), h, w};
      case Direction::east:
         return View{pos(w-1, h-1), delta( 0, -1), delta(-1,  0), w, h};
      case Direction::south:
         return View{pos(  0, h-1), delta( 1,  0), delta( 0, -1), h, w};
         // clang-format on
      }
   }

   size_t hash() const
   {
      return std::hash<std::string_view>{}(std::string_view(m_data.data(), m_data.size()));
   }

   void slide(Direction dir)
   {
      for (auto row : view(dir))
         row.slide();
   }

   size_t weight(Direction dir)
   {
      return accumulate(view(dir) | transform(&Row::weight), size_t{0});
   }

   void dump(Direction direction = Direction::west)
   {
      for (auto row : view(direction))
         fmt::println("{}", fmt::join(row, ""));
      fmt::println("");
   }

   const size_t w, h, dy;
   std::vector<char> m_data;
};

//
// This implementation relies on a non-[.#O]-border around the map, avoiding a lot of checks
// against vec.end().
//
void Row::slide()
{
   for (auto p0 = begin(); p0 != end();)
   {
      //
      //    OO.O.O..#.O
      // p0 ^
      //
      while (*p0 != 0 && *p0 != '.')
         ++p0;

      //
      //    OO.O.O..#.O
      // p0   ^
      // p1
      auto p1 = p0;
      while (*p1 == '.')
         ++p1;

      //
      //    OO.O.O..#.O
      // p0   ^
      // p1    ^
      //
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

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   std::vector<std::string> rows;
   for (std::string row; std::getline(file, row) && !row.empty();)
      rows.emplace_back(row);

   const auto w = rows[0].size(), h = rows.size();
   Map map(w, h);
   for (size_t i = 0; i < rows.size(); ++i)
   {
      assert(rows[i].size() == w);
      memcpy(map.pos(0, i), rows[i].data(), w);
   }

   //
   // part A: slide north once
   //
   map.slide(Direction::north);

   // dump(map);

   size_t A = map.weight(Direction::north);
   fmt::println("A: {}", A);

   //
   // part B: perforam 1M cycles -- which takes forever, but there are loops. Detect by hashing.
   //
   std::size_t t0 = 0, h0 = 0, loop_size = 0;
   std::set<size_t> hashes;
   const size_t CYCLES = 1'000'000'000;
   for (size_t cycle = 0; cycle < CYCLES; ++cycle)
   {
      //
      // perform a cycle of sliding N, W, S, E
      //
      for (auto dir : {Direction::north, Direction::west, Direction::south, Direction::east})
         map.slide(dir);

      //
      // detect loops by keeping a set of hashes
      //
      auto hash = map.hash();
      if (!hashes.insert(hash).second && !t0) // have we seen this hash already?
      {
         t0 = cycle, h0 = hash;
         fmt::println("start of loop: {}", t0);
      }
      else if (h0 == hash && !loop_size) // ... and another time?
      {
         loop_size = cycle - t0;
         fmt::println("end of loop at {}, size {}", cycle, cycle - t0);
         size_t forward_loops = (CYCLES - cycle) / loop_size;
         cycle += forward_loops * loop_size;
         fmt::println("forwarded {} loops to {}", forward_loops, cycle);
      }
      else if (h0 == hash)
         assert((cycle - t0) % loop_size == 0); // sanity check, but usually not reached
   }

   map.dump();
   size_t B = map.weight(Direction::north);
   fmt::println("A: {} B: {}", A, B);
}
