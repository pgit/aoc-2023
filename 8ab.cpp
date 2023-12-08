#include <cassert>
#include <fstream>
#include <string>
#include <string_view>
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

#include <fmt/ostream.h>

struct Node;
using NodePtr = std::unique_ptr<Node>;
struct Node
{
   std::string id;
   Node* left = nullptr;
   Node* right = nullptr;
   std::set<size_t> visited;
   bool operator<(const Node& r) const noexcept { return id < r.id; }
   bool operator<(const std::string& rid) const noexcept { return id < rid; }
};

struct NodeCmp
{
   using is_transparent = void;
   bool operator()(const NodePtr& l, const NodePtr& r) const { return *l < *r; };
   bool operator()(const NodePtr& l, const std::string& r) const { return *l < r; };
   bool operator()(const std::string& r, const NodePtr& l) const { return *l < r; };
};

std::optional<std::string> getline(std::ifstream& file)
{
   std::string line;
   if (std::getline(file, line))
      return line;
   else
      return std::nullopt;
}

int main(int argc, char* argv[])
{
   assert(argc == 2);
   std::ifstream file(argv[1]);

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

   auto distance = [&](Node* node, Node* to) -> size_t
   {
      size_t result = 0;
      while (node != to)
      {
         auto d = instructions[result++ % instructions.size()];
         node = d == 'L' ? node->left : node->right;
      }
      return result;
   };

   //
   // part A
   //
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

      size_t zstep = 0;
      size_t zcount = 0;
      size_t step = 0;
      auto* node = start;
      for (; !node->visited.contains(step % instructions.size()); ++step)
      {
         if (node->id[2] == 'Z')
         {
            assert(zcount == 0);
            zcount++;
            zstep = step;
         }
         node->visited.emplace(step % instructions.size());
         auto d = instructions[step % instructions.size()];
         node = d == 'L' ? node->left : node->right;
      };

      auto distance_to_start_of_loop = distance(start, node);

      // Why does this assertion hold? Seems to be due to the way the puzzle input is constructed...
      assert(step == distance_to_start_of_loop + zstep);
      assert(zcount == 1);

      fmt::println("{} loop at {} ({} -> {})", start->id, node->id,
                   distance_to_start_of_loop, node->visited.size());
      loops.emplace_back(start, distance_to_start_of_loop, step - distance_to_start_of_loop);
   }

   for (auto loop : loops)
      fmt::println("{} distance={} loop_size={}", loop.start->id, loop.distance_to_loop,
                   loop.loop_size);

   // I don't know why, but the solution is just the LCM of the loop sizes, despite initial path
   auto loop_sizes = loops | transform([](auto& info) { return info.loop_size; });
   B = boost::integer::lcm_range(loop_sizes.begin(), loop_sizes.end()).first;
#endif

   fmt::println("steps B: {}", B);
}
