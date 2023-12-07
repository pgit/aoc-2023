#include <cassert>
#include <cmath>
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

constexpr auto to_number = transform([](auto s) { return std::stol(s.str()); });
constexpr auto to_value = transform([](auto s) { return "23456789TJQKA"sv.find(s.str()[0]); });
constexpr auto to_valueA = transform([](auto c) { return "..23456789TJQKA"sv.find(c); });
constexpr auto to_valueB = transform([](auto c) { return "J.23456789T.QKA"sv.find(c); });

enum class Type : size_t
{
   five_of_a_kind = 7,
   four_of_a_kind = 6,
   full_house = 5,
   three_of_a_kind = 4,
   two_pair = 3,
   one_pair = 2,
   high_card = 1
};

struct CardInfo
{
   size_t card;
   long count = 0;
};

struct Hand
{
   Hand(std::string handString_, long bid_)
      : handString(std::move(handString_)), bid(bid_),
        hand(handString | to_valueA | to<std::vector>()),
        handB(handString | to_valueB | to<std::vector>()), //
        histA(hand | to<std::set>),
        histB(handB | filter([](auto c) { return c != 0; }) | to<std::set>), //
        infosA(computeInfos(histA)),
        infosB(computeInfos(histB)),
        typeA(computeType()), typeB(computeTypeWithJokers())
   {
   }

   const std::string handString;
   const long bid = 0;

   const std::vector<size_t> hand;
   const std::vector<size_t> handB;

   const std::set<size_t> histA;
   const std::set<size_t> histB;
   const std::vector<CardInfo> infosA;
   const std::vector<CardInfo> infosB;
   const Type typeA;
   const Type typeB;

   constexpr auto operator<=>(const Hand& r) const noexcept
   {
      auto& l = *this;
      return std::tie(l.typeA, l.hand[0], l.hand[1], l.hand[2], l.hand[3], l.hand[4]) <=>
             std::tie(r.typeA, r.hand[0], r.hand[1], r.hand[2], r.hand[3], r.hand[4]);
   }

   constexpr bool lessB(const Hand& r) const noexcept
   {
      auto& l = *this;
      return std::tie(l.typeB, l.handB[0], l.handB[1], l.handB[2], l.handB[3], l.handB[4]) <
             std::tie(r.typeB, r.handB[0], r.handB[1], r.handB[2], r.handB[3], r.handB[4]);
   }

private:
   std::vector<CardInfo> computeInfos(const std::set<size_t>& hist)
   {
      auto infos = hist |
                   transform(
                      [&](size_t card) {
                         return CardInfo{card, count(hand, card)};
                      }) |
                   to<std::vector>;
      sort(infos, std::less<>(), &CardInfo::count);
      return infos;
   }

   Type computeType()
   {
      if (histA.size() == 1)
         return Type::five_of_a_kind; // 5
      else if (histA.size() == 2)
      {
         if (infosA[1].count == 4)
            return Type::four_of_a_kind; // 1, 4
         else
            return Type::full_house; // 2, 3
      }
      else if (histA.size() == 3)
      {
         if (infosA[2].count == 3)
            return Type::three_of_a_kind; // 1, 1, 3
         else
            return Type::two_pair; // 1, 2, 2
      }
      else if (histA.size() == 4)
         return Type::one_pair; // 1, 1, 1, 2
      else
         return Type::high_card; // 1, 1, 1, 1, 1
   }

   Type computeTypeWithJokers()
   {
      auto jokers = count(handB, 0);
      if (histB.size() <= 1)  // may also be all jokers
         return Type::five_of_a_kind; // 5
      else if (histB.size() == 2)
      {
         if (infosB[1].count + jokers == 4)
            return Type::four_of_a_kind; // 1, 4
         else
            return Type::full_house; // 2, 3
      }
      else if (histB.size() == 3)
      {
         if (infosB[2].count + jokers == 3)
            return Type::three_of_a_kind; // 1, 1, 3
         else
            return Type::two_pair; // 1, 2, 2
      }
      else if (histB.size() == 4)
         return Type::one_pair; // 1, 1, 1, 2
      else
         return Type::high_card; // 1, 1, 1, 1, 1
   }
};

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

   std::vector<std::unique_ptr<Hand>> hands;

   std::string line;
   while (std::getline(file, line))
   {
      boost::smatch what;
      static const boost::regex regexp{R"((\w+) (\d+))"};
      boost::regex_match(line, what, regexp, boost::match_extra | boost::match_perl);
      hands.emplace_back(std::make_unique<Hand>(what[1].str(), std::stol(what[2])));
   }

   //
   // part A
   //
   std::ranges::sort(hands, [](auto& a, auto& b) { return *a < *b; });

   for (auto& hand : hands)
      fmt::println("{} {} [{}] type={}", hand->handString, hand->bid, fmt::join(hand->hand, ", "),
                   size_t(hand->typeA));

   size_t A = 0;
   for (auto [hand, rank] : zip(hands, iota(1)))
      A += hand->bid * rank;

   fmt::println("A={}", A);

   //
   // part B
   //
   std::ranges::sort(hands, [](auto& a, auto& b) { return a->lessB(*b); });

   for (auto& hand : hands)
      fmt::println("{} {} [{}] type={} hist=[{}]", hand->handString, hand->bid, fmt::join(hand->hand, ", "),
                   size_t(hand->typeB), fmt::join(hand->histB, ", "));

   size_t B = 0;
   for (auto [hand, rank] : zip(hands, iota(1)))
      B += hand->bid * rank;

   fmt::println("B={}", B);
}
