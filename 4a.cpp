#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <string>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

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

constexpr auto to_number = transform([](auto s) { return std::stoi(s.str()); });

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   std::deque<size_t> lookahead;
   size_t sum = 0;
   size_t total_cards = 0;
   const boost::regex regexp{R"(Card +(\d+):(?: +(\d+))+ \|(?: +(\d+))+)"};
   for (std::string line; std::getline(file, line);)
   {
      boost::smatch what;
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);

      //
      // parse
      //
      int card_number = std::stoi(what[1].str());
      auto winning_numbers = what[2].captures() | to_number | ranges::to<std::set>;
      auto my_numbers = what[3].captures() | to_number | ranges::to<std::set>;

      //
      // part a: number of matches == set intersection size
      //
      std::vector<decltype(winning_numbers)::value_type> matches;
      std::ranges::set_intersection(winning_numbers, my_numbers, std::back_inserter(matches));

      if (!matches.empty())
         sum += 1 << (matches.size() - 1);

      //
      // part b: apply copies stored in look-ahead deque
      //
      total_cards++;

      // numbers of copies of the current card
      size_t copies = 1;
      if (!lookahead.empty())
      {
         total_cards += lookahead.front();
         copies += lookahead.front();
         lookahead.pop_front();
      }

      // apply 'copies' to the next 'matches.size()' cards via lookahead
      while (lookahead.size() < matches.size())
         lookahead.emplace_back();

      for (auto [extra, _] : zip(lookahead, iota(0ul, matches.size())))
         extra += copies;

      fmt::println("Card {}: {} matches, {} copies, lookahead=[{}]", card_number, matches.size(),
                   copies, fmt::join(lookahead, ", "));
   }

   fmt::println("solution: {} total cards: {}", sum, total_cards);
}
