//
// https://adventofcode.com/2023/day/18
//
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include "range/v3/view/iota.hpp"
#include "range/v3/view/zip.hpp"
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include "common.hpp"

struct Coord
{
   int x = 0, y = 0;
   Coord operator-() const { return Coord{-x, -y}; }
   Coord transpose() const { return Coord{y, x}; }
   Coord operator+(const Coord& r) const { return Coord{x + r.x, y + r.y}; }
   Coord operator-(const Coord& r) const { return Coord{x - r.x, y - r.y}; }
   Coord operator*(int f) const { return Coord{x * f, y * f}; }
   const Coord& operator+=(const Coord& r)
   {
      x += r.x;
      y += r.y;
      return *this;
   }
   Coord min(const Coord& r) { return {std::min(x, r.x), std::min(y, r.y)}; }
   Coord max(const Coord& r) { return {std::max(x, r.x), std::max(y, r.y)}; }
   bool operator==(const Coord& r) const noexcept = default;
};

struct Line
{
   Coord p0, p1;
};

struct Tile
{
   char symbol = '.';
   uint32_t color = 0x00001f;
};

static std::array<int, 10> HEAT{16, 18, 20, 56, 91, 160, 166, 178, 184, 229};
static const std::array<Coord, 4> AROUND = {{{-1, 0}, {0, -1}, {1, 0}, {0, 1}}};

const std::map<char, std::string_view> LINE_DRAWING = {
   {'.', "."}, {'-', "─"}, {'|', "│"}, {'L', "└"}, {'J', "┘"}, {'7', "┐"}, {'F', "┌"}};

const std::map<char, Coord> DIRECTIONS = {
   {'L', Coord{-1, 0}},
   {'U', Coord{0, -1}},
   {'R', Coord{1, 0}},
   {'D', Coord{0, 1}},
};

struct Map
{
   Map(int w, int h)
   {
      for (auto y : iota(0, h))
         map.emplace_back(w, Tile{});
   }
   explicit Map(std::ifstream file)
   {
      for (std::string line; std::getline(file, line);)
         map.emplace_back(line | transform([](char c) { return Tile{c}; }) | to<std::vector>);

      for (auto&& row : map)
         assert(row.size() == width());
   }

   std::vector<std::vector<Tile>> map;

   int width() const { return map[0].size(); }
   int height() const { return map.size(); }

   Tile& at(const Coord& c)
   {
      if (c.y >= 0 && c.y < map.size())
         if (c.x >= 0 && c.x <= map[c.y].size())
            return map[c.y][c.x];

      static Tile WALL = {'#'};
      return WALL; // yes I know I'm returning a non-const reference to a static here
   }

   void dump() const
   {
      for (auto& row : map)
      {
         for (auto& cell : row)
         {
            auto c = cell.color;
            auto symbol = LINE_DRAWING.at(cell.symbol);
            fmt::print("\x1b[48;2;{};{};{}m{}\x1b[0m", c >> 16, (c >> 8) & 255, c & 255, symbol);
         }
         fmt::println("");
      }
   }

   void trace(Coord pos, Coord dir, int run)
   {
      // TODO:
   }
};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   Coord pos;
   Coord min{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
   Coord max{std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};

   //
   // Go through input once to determine map size.
   // For part B, record map of horizontal lines.
   //
   const boost::regex regexp{R"(([LRUD]) ([0-9]+) \(#([a-zA-Z0-9]+)\))"};
   for (std::string line; std::getline(file, line);)
   {
      boost::smatch what;
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
      auto dir = DIRECTIONS.at(what[1].str()[0]);
      auto steps = std::stoi(what[2].str());
      pos += dir * steps;
      min = min.min(pos);
      max = max.max(pos);
   }
   fmt::println("min=({}, {}) max=({}, {})", min.x, min.y, max.x, max.y);

   //
   // Create map and plot path. Determine corner shape ('7', 'J', 'F' or 'L') as in Day 10.
   //
   size_t A = 0;
   file.clear();
   file.seekg(0);
   pos = -min;
   Map map{max.x - min.x + 1, max.y - min.y + 1};
   map.at(pos).symbol = 'F'; // FIXME: might be different, but matched example and puzzle
   Coord last_dir;
   for (std::string line; std::getline(file, line);)
   {
      boost::smatch what;
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
      auto dir = DIRECTIONS.at(what[1].str()[0]);
      auto steps = std::stoi(what[2].str());
      auto color = std::stol(what[3].str(), nullptr, 16);

      // clang-format off
      if      (last_dir.x ==  1) map.at(pos).symbol = dir.y == 1 ? '7' : 'J';
      else if (last_dir.y ==  1) map.at(pos).symbol = dir.x == 1 ? 'L' : 'J';
      else if (last_dir.x == -1) map.at(pos).symbol = dir.y == 1 ? 'F' : 'L';
      else if (last_dir.y == -1) map.at(pos).symbol = dir.x == 1 ? 'F' : '7';
      map.at(pos).color = color;
      pos += dir;
      // clang-format on

      for (auto step : iota(1, steps))
      {
         map.at(pos).symbol = dir.x ? '-' : '|';
         map.at(pos).color = color;
         pos += dir;
      }
      A += steps;
      last_dir = dir;
   }
   assert(pos == -min);

   //
   // Same as Day 10, determine blocks that are inside by horizontal scan line algorithm.
   //
   for (auto&& [row, y] : zip(map.map, iota(0)))
   {
      bool inside = false;
      for (auto it = row.begin(); it != row.end(); ++it)
      {
         switch (it->symbol)
         {
         case '|':
            inside = !inside;
            break;

         case '.':
            if (inside)
            {
               it->color = 0xff0000;
               A++;
            }
            break;

         case 'F':
         case 'L':
         {
            auto start = *it++;
            while (it->symbol == '-') // skip ┄
               it++;
            if (start.symbol == 'F' && it->symbol == 'J' || // ┌┄┘
                start.symbol == 'L' && it->symbol == '7') // └┄┐
               inside = !inside;
         }
         break;

         default:
            fmt::println("unexpected symbol '{}'", it->symbol);
            assert(false);
         }
      }
   }

   map.dump();
   fmt::println("A: {}", A);
}
