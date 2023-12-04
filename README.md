# Adevent of Code 2023
These are my attempts at [Advent of Code 2023](https://adventofcode.com/2023).

I'm doing this in C++, my main language.

Usually, I just try to get to the solution quick and dirty, but practicing with ranges while I'm at it. Also, it's demonstrating how to work in a [VSCode Dev container](https://code.visualstudio.com/docs/devcontainers/containers).

## Day 4
I like this one because there is a nice feature of [boost::regex](https://www.boost.org/doc/libs/1_83_0/libs/regex/doc/html/index.html) allowing a capture to happen multiple times, if that capture is part of a repeated sub-expression.

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
