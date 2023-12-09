# Adevent of Code 2023
These are my attempts at [Advent of Code 2023](https://adventofcode.com/2023).

I'm doing this in C++, my main language.

Usually, I just try to get to the solution quick and dirty, but practicing with ranges while I'm at it. Also, it's demonstrating how to work in a [VSCode Dev container](https://code.visualstudio.com/docs/devcontainers/containers).

## [Day 3](https://adventofcode.com/2023/day/3) [(code)](src/3ab.cpp)

Added empty rows and columns around the actual input array, to avoid the proverbial border cases.

## [Day 4](https://adventofcode.com/2023/day/4) [(code)](src/4ab.cpp)

I like [this one](src/4ab.cpp) because there is a nice feature of [boost::regex](https://www.boost.org/doc/libs/1_83_0/libs/regex/doc/html/index.html) allowing a capture to happen multiple times, if that capture is part of a repeated sub-expression.

```c++
boost::regex regexp{R"(Card +(\d+):(?: +(\d+))+ \|(?: +(\d+))+)"};
boost::smatch what;
boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
auto winning_numbers = what[2].captures() |
   transform([](auto s) { return std::stoi(s.str()); }) |
   ranges::to<std::set>;
```

Note: For the objectives, the actual numeric values on the cards are never used, so we wouldn't even need to to do `stoi` here.

The inner `(\d+)` capture group is repeated because it is embeded in a non-capturing group `(?: +(\d+))+`, which maches `" 1 12 13 14"` four times and can be extracted using by calling `captures()` on the match. That object has `begin()` and `end()` functions and interacts nicely with ranges.

Also, for the second part, the extra copies of the cards are handled using a `std::deque` look-ahead in an efficient manner.

## [Day 7](https://adventofcode.com/2023/day/7) [(code)](src/7ab.cpp)

Clean core function to compute the hand 'type': Note that the `hand` is already sorted according to card value:

```c++
// '2' => 2, ..., 'T' => 10, ...
auto to_valueA = transform([](auto c) -> int { return "..23456789TJQKA"sv.find(c); });

// 'J' => 0, '2' => 2, ...
auto to_valueB = transform([](auto c) -> int { return "J.23456789T.QKA"sv.find(c); });
```

 For Part II, the Joker gets special value 0.
```c++
Type computeType()
{
   // hand   = [14, 2, 0, 14, 12]

   auto unique = hand | filter([](auto c) { return c != 0; }) | to<std::set>;
   auto hist = unique | transform([&](int card) { return count(hand, card); }) | to<std::vector>;
   sort(hist);
   auto jokers = count(hand, 0);

   // unique = {2, 14, 12}
   // hist   = [1, 1, 2]
   // jokers = 1

   if (unique.size() <= 1) // may also be all jokers
      return Type::five_of_a_kind; // 5
   else if (unique.size() == 2)
   {
      if (hist[1] + jokers == 4)
         return Type::four_of_a_kind; // 1, 4
      else
         return Type::full_house; // 2, 3
   }
   else if (unique.size() == 3)
   {
      if (hist[2] + jokers == 3)
         return Type::three_of_a_kind; // 1, 1, 3
      else
         return Type::two_pair; // 1, 2, 2
   }
   else if (unique.size() == 4)
      return Type::one_pair; // 1, 1, 1, 2
   else
      return Type::high_card; // 1, 1, 1, 1, 1
}
```

## [Day 8](https://adventofcode.com/2023/day/8) [(code)](src/8ab.cpp)

As [others on reddit](https://www.reddit.com/r/adventofcode/comments/18df7px/2023_day_8_solutions/), I did not like part II of today very much. I think the input is constructed in a very specific, non-general way for the solution to work as it does.

Anyway, I learned about [Boost.Integer LCM](https://www.boost.org/doc/libs/1_83_0/libs/integer/doc/html/boost_integer/gcd_lcm.html) and notably `lcm_range`, which computes LCM not just of two integeres, but a whole range.

## [Day 9](https://adventofcode.com/2023/day/9) [(code)](src/9ab.cpp)

Finally, an easy one again. Things I learned today:

With [ranges::views::istream](https://ericniebler.github.io/range-v3/istream_8hpp.html), you can easily parse a sequence of numbers, with less overhead than the regexp method mentioned before.
```c++
std::istringstream ss{line};
auto numbers = istream<long>(ss) | ranges::to<std::vector>;
```
This is also possible with the file stream directly, but that would ignore the newlines. And as with most AoC code, no error checking.
