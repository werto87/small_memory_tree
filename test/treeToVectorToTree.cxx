#include "small_memory_tree/treeToVector.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <range/v3/algorithm.hpp>
#include <range/v3/range_fwd.hpp>
#include <st_tree.h>
#include <sys/types.h>
#include <type_traits>
#include <vector>

// TODO maybe instead of index use relative position index is limited to the type size. relative position is kinda limited to type size but not that strong
// index in unsigned char max allowed size of vector is 253 with relative position max distance between parent and the furthest child smaller 253.

TEST_CASE ("2 children", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  auto myVec = treeToVector (tree, 255, 254);
  for (auto &value : childrenByPath (myVec, { 2, 4 }, 255, 2))
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
  REQUIRE (vectorToTree (treeToVector (tree, 255, 254), 255, 2) == tree);
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
  for (auto &value : childrenByPath (myVec, { { 2, 2 }, { 4, 4 } }, { 255, -1 }, 3))
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
  REQUIRE (vectorToTree (treeToVector (tree, std::tuple<uint8_t, int8_t>{ 255, -1 }, std::tuple<uint8_t, int8_t>{ 254, -1 }), std::tuple<uint8_t, int8_t>{ 255, -1 }, 3) == tree);
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
  auto result = childrenByPath (myVec, { { 0, Result::Undefined }, { 0, Result::Undefined } }, { 255, Result::Undefined }, 3);
  REQUIRE_FALSE (result.empty ());
  REQUIRE (result.at (0) == std::tuple<uint8_t, Result>{ 1, Result::Undefined });
}