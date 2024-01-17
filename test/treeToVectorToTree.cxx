#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/treeToVector.hxx"
#include "small_memory_tree/vectorToTree.hxx"
#include <catch2/catch.hpp>
#include <cstdint>
#include <iterator>
#include <st_tree.h>

using namespace small_memory_tree;

TEST_CASE ("vectorToTree tree to vector", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2000);
  tree.root ().insert (3000);
  tree.root ()[0].insert (4000);
  tree.root ()[0][0].insert (42000);
  tree.root ()[1].insert (42000);
  tree.root ()[1][0].insert (42000);
  tree.root ()[1][0][0].insert (123000);
  auto maxIntValue = std::numeric_limits<int>::max ();
  auto myVec = treeToVector (tree, maxIntValue, maxIntValue - 1);
  REQUIRE (vectorToTree (myVec) == tree);
}

TEST_CASE ("3 children tree to vector", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  for (auto i = 0; i < 15; ++i)
    {
      tree.root ().insert (11);
    }
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  REQUIRE (vectorToTree (myVec) == tree);
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
  REQUIRE (vectorToTree (myVec) == tree);
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
  auto myVec = treeToVector (tree, std::tuple<uint8_t, Result>{ 255, Result::Undefined }, std::tuple<uint8_t, Result>{ 254, Result::Undefined }, [] (auto const &node) {
    //
    return std::tuple<uint8_t, Result>{ node.key ().value (), std::get<0> (node.data ()) };
  });
  REQUIRE (myVec.size () == 24);
  auto result = childrenByPath (myVec, { { 0, Result::Undefined }, { 0, Result::Undefined } });
  REQUIRE_FALSE (result.empty ());
  REQUIRE (result.at (0) == std::tuple<uint8_t, Result>{ 1, Result::Undefined });
}

TEST_CASE ("treeToVector value equals marker for empty during child distance calculation", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  for (size_t i = 0; i < 15; ++i)
    {
      tree.root ().insert (42);
    }
  REQUIRE_THROWS_AS (treeToVector (tree, uint8_t{ 225 }, uint8_t{ 254 }), std::logic_error);
}

TEST_CASE ("treeToVector too big tree", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  for (size_t i = 0; i < 16; ++i)
    {
      tree.root ().insert (42);
    }
  REQUIRE_THROWS_AS (treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 }), std::logic_error);
}

TEST_CASE ("treeToVector value equals marker for child", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  for (size_t i = 0; i < 16; ++i)
    {
      tree.root ().insert (254);
    }
  REQUIRE_THROWS_AS (treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 }), std::logic_error);
}

TEST_CASE ("treeToVector value equals marker for empty", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  for (size_t i = 0; i < 16; ++i)
    {
      tree.root ().insert (254);
    }
  REQUIRE_THROWS_AS (treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 }), std::logic_error);
}