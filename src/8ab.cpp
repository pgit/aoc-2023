#include <string>
using namespace std::literals;

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
using namespace ranges;
using namespace ranges::views;

#define BOOST_REGEX_MATCH_EXTRA
#include <boost/regex.hpp>

#include <boost/integer/common_factor_rt.hpp>

#include "common.hpp"

struct Node;
using NodePtr = std::unique_ptr<Node>;
struct Node
{
   std::string id;
   Node* left = nullptr;
   Node* right = nullptr;
   std::set<size_t> visited;
};

struct NodeCmp
{
   using is_transparent = void;
   bool operator()(const NodePtr& l, const NodePtr& r) const { return l->id < r->id; };
   bool operator()(const NodePtr& l, const std::string& r) const { return l->id < r; };
   bool operator()(const std::string& r, const NodePtr& l) const { return r < l->id; };
};

int main(int argc, char* argv[])
{
   auto file = input(argc, argv);

   std::set<NodePtr, NodeCmp> nodes;

   auto instructions = *getline(file);
   fmt::println("instructions: {}", instructions);
   getline(file);

   static const boost::regex regexp{R"((\w+) = \((\w+), (\w+)\))"};
   while (auto line = getline(file))
   {
      boost::smatch what;
      boost::regex_match(*line, what, regexp, boost::match_extra | boost::match_perl);
      nodes.insert(std::make_unique<Node>(what[1].str()));
   }
   file.clear();
   file.seekg(0);
   getline(file);
   getline(file);
   while (auto line = getline(file))
   {
      boost::smatch what;
      boost::regex_match(*line, what, regexp, boost::match_extra | boost::match_perl);
      auto& node = *nodes.find(std::string(what[1].str()));
      node->left = nodes.find(what[2].str())->get();
      node->right = nodes.find(what[3].str())->get();
   }

   for (auto& node : nodes)
      fmt::println("{} -> ({}, {})", node->id, node->left->id, node->right->id);

   //
   // part A
   //
   auto distance = [&](Node* node, Node* destination) -> size_t
   {
      size_t steps = 0;
      for (size_t i; i = steps % instructions.size(), node != destination; ++steps)
         node = instructions[i] == 'L' ? node->left : node->right;
      return steps;
   };

   size_t A = distance(nodes.find("AAA")->get(), nodes.find("ZZZ")->get());
   fmt::println("steps A: {}", A);

   //
   // part B (brute force / LCM)
   //
   size_t B = 0;
#if 0
   auto iterators = nodes | filter([](auto& e) { return e->id[2] == 'A'; }) |
                    transform([](auto& e) { return e.get(); }) | to<std::vector>;                   
   while (!all_of(iterators, [](auto& e) { return e->id[2] == 'Z'; }))
   {
      fmt::println("{} {}", B,
                   fmt::join(iterators | transform([](const auto& e) { return e->id; }), ", "));
      auto d = instructions[B++ % instructions.size()];
      for (auto& node : iterators)
         node = d == 'L' ? node->left : node->right;
   }
#else
   struct LoopInfo
   {
      Node* start;
      size_t distance_to_loop, loop_size;
   };
   std::vector<LoopInfo> loops;

   for (auto start : nodes | filter([](auto& e) { return e->id[2] == 'A'; }) |
                        transform([](auto& e) { return e.get(); }))
   {
      for (auto& node : nodes)
         node->visited.clear();

      size_t step = 0;
      auto* node = start;
      for (size_t i; i = step % instructions.size(), !node->visited.contains(i); ++step)
      {
         node->visited.emplace(i);
         node = instructions[i] == 'L' ? node->left : node->right;
      };

      auto distance_to_loop = distance(start, node);
      auto loop_size = step - distance_to_loop;

      auto* dest = start;
      for (size_t i = 0; i < loop_size; ++i)
         dest = instructions[i] == 'L' ? dest->left : dest->right;

      fmt::println("{} loop at {}, visited={}, distance_to_loop={}, loop_size={} --> {}", start->id,
                   node->id, node->visited.size(), distance_to_loop, loop_size, dest->id);
      loops.emplace_back(start, distance_to_loop, loop_size);
   }

   // I don't know why, but the solution is just the LCM of the loop sizes, despite initial path
   auto loop_sizes = loops | transform([](auto& info) { return info.loop_size; });
   B = boost::integer::lcm_range(loop_sizes.begin(), loop_sizes.end()).first;
#endif

   fmt::println("steps B: {}", B);
}
