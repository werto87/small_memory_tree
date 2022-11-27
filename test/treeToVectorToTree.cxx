#include "small_memory_tree/treeToVector.hxx"
#include <catch2/catch.hpp>
#include <cstdint>
#include <iterator>
#include <range/v3/algorithm.hpp>
#include <range/v3/range_fwd.hpp>
#include <st_tree.h>

// TODO maybe instead of index use relative position index is limited to the type size. relative position is kinda limited to type size but not that strong
// index in unsigned char max allowed size of vector is 253 with relative position max distance between parent and the furthest child smaller 253.
using namespace small_memory_tree;

TEST_CASE ("2 children", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  auto myVec = treeToVector (tree, 255, 254);
  for (auto &value : childrenByPath (myVec, { 2, 4 }, 255))
    {
      REQUIRE (value == 42);
    }
}

TEST_CASE ("vectorToTree tree to vector", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  tree.root ()[1].insert (42);
  tree.root ()[1][0].insert (42);
  tree.root ()[1][0][0].insert (123);
  auto myVec = treeToVector (tree, 255, 254);
  REQUIRE (vectorToTree (myVec, 255) == tree);
}

TEST_CASE ("3 children and tuple", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto myVec = treeToVector (tree, std::tuple<uint8_t, int8_t>{ 255, -1 }, std::tuple<uint8_t, int8_t>{ 254, -1 });
  for (auto &value : childrenByPath (myVec, { { 2, 2 }, { 4, 4 } }, { 255, -1 }))
    {
      REQUIRE (value == std::tuple<uint8_t, int8_t>{ 42, 42 });
    }
}

TEST_CASE ("3 children and tuple vectorToTree tree to vector", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto myVec = treeToVector (tree, std::tuple<uint8_t, int8_t>{ 255, -1 }, std::tuple<uint8_t, int8_t>{ 254, -1 });
  REQUIRE (vectorToTree (myVec, std::tuple<uint8_t, int8_t>{ 255, -1 }) == tree);
}

TEST_CASE ("treeToVector more than 255 elements", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node = node->insert (i);
    }
  //  TODO there should be an error message if treeToVector is just used with 255 and 254 because this are implicit ints and not uint8_t
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  REQUIRE (myVec.size () == 264);
  //  int{} so it is easier to read the error message when the test fails
  REQUIRE (int{ myVec.at (253) } != int{ 254 });
}

TEST_CASE ("treeToVector more than 255 elements second child has 2 children first child no children", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (42);
  for (uint8_t i = 43; i < 130;)
    {
      node->insert (i);
      ++i;
      node = node->insert (i);
      ++i;
    }
  //  TODO there should be an error message if treeToVector is just used with 255 and 254 because this are implicit ints and not uint8_t
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  REQUIRE (myVec.size () == 270);
  //  int{} so it is easier to read the error message when the test fails
  REQUIRE (int{ myVec.at (263) } == int{ 4 });
}

enum class Result : uint8_t
{
  Undefined,
  DefendWon,
  Draw,
  AttackWon
};

class Action
{
public:
  Action () = default;
  Action (std::uint8_t cardPlayed_) : cardPlayed (cardPlayed_) {}
  auto operator<=> (const Action &) const = default;

  std::uint8_t
  value () const
  {
    return cardPlayed;
  };

private:
  std::uint8_t cardPlayed{ 253 };
};

TEST_CASE ("treeToVector", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<Result, bool>, st_tree::keyed<Action> >{};
  tree.insert ({ Result::Undefined, true });
  tree.root ().insert (0, { Result::Undefined, true });
  tree.root ().insert (2, { Result::Undefined, true });
  tree.root ().insert (3, { Result::Undefined, true });
  tree.root ()[0].insert (0, { Result::Undefined, true });
  tree.root ()[0][0].insert (1, { Result::Undefined, true });
  auto myVec = treeToVector (tree, std::tuple<uint8_t, Result>{ 255, Result::Undefined }, std::tuple<uint8_t, Result>{ 254, Result::Undefined }, [] (auto const &node) { return std::tuple<uint8_t, Result>{ node.key ().value (), std::get<0> (node.data ()) }; });
  REQUIRE (myVec.size () == 24);
  auto result = childrenByPath (myVec, { { 0, Result::Undefined }, { 0, Result::Undefined } }, { 255, Result::Undefined });
  REQUIRE_FALSE (result.empty ());
  REQUIRE (result.at (0) == std::tuple<uint8_t, Result>{ 1, Result::Undefined });
}