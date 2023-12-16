//
// https://adventofcode.com/2023/day/13
//
#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <deque>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/fold_left.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include "range/v3/view/iota.hpp"
#include "range/v3/view/zip.hpp"
using namespace ranges;
using namespace ranges::views;

#include "common.hpp"

template <class T>
T to(const std::string_view& input)
{
   T out;
   auto result = std::from_chars(input.data(), input.data() + input.size(), out);
   return out;
}

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   std::string line;
   std::getline(file, line);
   assert(!line.empty());

   auto steps = line | split(',') | transform([](auto&& rng) { //
                   return std::string_view(&*rng.begin(), ranges::distance(rng));
                });

   //
   // part A
   //
   auto rangeA = steps | transform([](auto&& sv) { //
                    return fold_left(sv, 0, [](auto a, auto c) -> uint8_t { //
                       return (a + c) * 17;
                    });
                 });

   size_t A = accumulate(rangeA, size_t{0});
   fmt::println("A: {}", A);

   //
   // part B
   //
   struct Label
   {
      std::string_view step;
      std::string_view label;
      uint8_t hash;
      std::optional<int> focal_length;
   };

   auto rangeB = steps | transform([](auto&& sv) -> Label { //
                    Label label{sv};
                    label.step = sv;
                    label.label = sv.substr(0, sv.find_last_of("=-"));
                    label.hash = fold_left(label.label, 0, [](auto a, auto c) -> uint8_t { //
                       return (a + c) * 17;
                    });
                    if (auto pos = sv.find_last_of('='); pos != std::string_view::npos)
                       label.focal_length = to<int>(sv.substr(pos + 1));
                    return label;
                 });

   std::array<std::deque<Label>, 256> boxes;
   for (auto step : rangeB)
   {
      auto& deque = boxes[step.hash];

      auto it = std::ranges::find(deque, step.label, &Label::label);
      if (step.focal_length && it != deque.end())
         *it = step;
      else if (step.focal_length)
         deque.emplace_back(step);
      else if (it != deque.end())
         deque.erase(it);
   }

   for (auto box : zip(boxes, iota(0)))
   {
      if (box.first.empty())
         continue;
      fmt::println("box {}: {}", box.second, fmt::join(box.first | transform(&Label::step), ", "));
   }

   size_t B = 0;
   for (auto [box, i] : zip(boxes, iota(1)))
      for (auto&& [lens, j] : zip(box, iota(1)))
         B += i * *lens.focal_length * j;

   fmt::println("A: {} B: {}", A, B);
}
