#include <cassert>
#include <deque>
#include <fstream>
#include <string>

#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include <fmt/ostream.h>

constexpr auto to_number = transform([](auto s) { return std::stoi(s.str()); });

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   std::deque<int> lookahead;
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
      auto winning_numbers = what[2].captures() | to_number | to<std::set>;
      auto my_numbers = what[3].captures() | to_number | to<std::set>;

      //
      // part a: number of matches == set intersection size
      //
      int matches = count_if(my_numbers, [&](auto n) { return winning_numbers.contains(n); });
      if (matches)
         sum += 1 << (matches - 1);

      //
      // part b: apply copies stored in look-ahead deque
      //
      // numbers of copies of the current card
      int copies = 1;
      if (!lookahead.empty())
      {
         copies += lookahead.front();
         lookahead.pop_front();
      }

      total_cards += copies;

      // apply 'copies' to the next 'matches.size()' cards via lookahead
      if (lookahead.size() < matches)
         lookahead.resize(matches);

      for (auto [extra, _] : zip(lookahead, iota(0, matches)))
         extra += copies;

      fmt::println("Card {}: {} matches, {} copies, lookahead=[{}]", card_number, matches, copies,
                   fmt::join(lookahead, ", "));
   }

   fmt::println("solution: {} total cards: {}", sum, total_cards);
}
