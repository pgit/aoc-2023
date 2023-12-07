#include <cassert>
#include <fstream>
#include <string>
#include <string_view>
using namespace std::literals;

#include <range/v3/algorithm/count.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include <fmt/ostream.h>

// '2' => 2, ..., 'T' => 10, ...
constexpr auto to_valueA = transform([](auto c) -> int { return "..23456789TJQKA"sv.find(c); });

// 'J' => 0, '2' => 2, ...
constexpr auto to_valueB = transform([](auto c) -> int { return "J.23456789T.QKA"sv.find(c); });

enum class Type : int
{
   five_of_a_kind = 7,
   four_of_a_kind = 6,
   full_house = 5,
   three_of_a_kind = 4,
   two_pair = 3,
   one_pair = 2,
   high_card = 1
};

struct Hand
{
   Hand(std::string handString_, std::vector<int> hand_, long bid_)
      : handString(std::move(handString_)), hand(std::move(hand_)), bid(bid_), type(computeType())
   {
   }

   const std::string handString;
   const std::vector<int> hand;
   const long bid = 0;
   const Type type;

   constexpr auto operator<=>(const Hand& r) const noexcept
   {
      return std::tie(type, hand) <=> std::tie(r.type, r.hand);
   }

private:
   Type computeType()
   {
      auto hist = hand | filter([](auto c) { return c != 0; }) | to<std::set>;
      auto infos = hist | transform([&](int card) { return count(hand, card); }) | to<std::vector>;
      sort(infos);
      auto jokers = count(hand, 0);

      if (hist.size() <= 1) // may also be all jokers
         return Type::five_of_a_kind; // 5
      else if (hist.size() == 2)
      {
         if (infos[1] + jokers == 4)
            return Type::four_of_a_kind; // 1, 4
         else
            return Type::full_house; // 2, 3
      }
      else if (hist.size() == 3)
      {
         if (infos[2] + jokers == 3)
            return Type::three_of_a_kind; // 1, 1, 3
         else
            return Type::two_pair; // 1, 2, 2
      }
      else if (hist.size() == 4)
         return Type::one_pair; // 1, 1, 1, 2
      else
         return Type::high_card; // 1, 1, 1, 1, 1
   }
};

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   std::vector<std::unique_ptr<Hand>> handsA;
   std::vector<std::unique_ptr<Hand>> handsB;

   std::string line;
   while (std::getline(file, line))
   {
      boost::smatch what;
      static const boost::regex regexp{R"((\w+) (\d+))"};
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
      auto hand = what[1].str();
      auto bid = std::stol(what[2]);
      handsA.emplace_back(std::make_unique<Hand>(hand, hand | to_valueA | to<std::vector>(), bid));
      handsB.emplace_back(std::make_unique<Hand>(hand, hand | to_valueB | to<std::vector>(), bid));
   }

   auto doit = [](std::vector<std::unique_ptr<Hand>>& hands)
   {
      std::ranges::sort(hands, [](auto& a, auto& b) { return *a < *b; });

      for (auto& hand : hands)
         fmt::println("{} {} [{}] type={}", hand->handString, hand->bid,
                      fmt::join(hand->hand, ", "), size_t(hand->type));

      int result = 0;
      for (auto [hand, rank] : zip(hands, iota(1)))
         result += hand->bid * rank;
      return result;
   };

   //
   // part A
   //
   fmt::println("A={}", doit(handsA));

   //
   // part B
   //
   fmt::println("B={}", doit(handsB));
}
