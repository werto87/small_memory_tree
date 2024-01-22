/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/smallMemoryTree.hxx"
#include "small_memory_tree/treeToVector.hxx"
#include <catch2/catch.hpp>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <st_tree.h>

using namespace small_memory_tree;

TEST_CASE ("vectorToTree tree to vector")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0][0].insert (4);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  internals::serialize_indented (tree.begin (), tree.end (), std::cout);
  std::cout << "MY TREE" << std::endl;
  auto treeFromVector = smt.generateTreeFromVector ();
  internals::serialize_indented (treeFromVector.begin (), treeFromVector.end (), std::cout);
  REQUIRE (smt.generateTreeFromVector () == tree);
}

TEST_CASE ("3 children tree to vector")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  for (auto i = 0; i < 15; ++i)
    {
      tree.root ().insert (11);
    }
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  REQUIRE (smt.generateTreeFromVector () == tree);
}

TEST_CASE ("3 children and tuple vectorToTree tree to vector")
{
  auto tree = st_tree::tree<std::tuple<int, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto myVec = treeToVector (tree, std::tuple<int, int8_t>{ 255, -1 });
  auto smt = SmallMemoryTree<std::tuple<int, int8_t> >{ tree, { 255, -1 } };
  REQUIRE (smt.generateTreeFromVector () == tree);
}

TEST_CASE ("treeToVector more than 255 elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (int i = 0; i < 130; ++i)
    {
      node = node->insert (i);
    }
  //  TODO there should be an error message if treeToVector is just used with 255 because this is implicit an int and not int
  auto myVec = treeToVector (tree, int{ 255 });
  REQUIRE (myVec.size () == 264);
  //  int{} so it is easier to read the error message when the test fails
  REQUIRE (int{ myVec.at (253) } != int{ 254 });
}

enum class Result : int
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

TEST_CASE ("treeToVector")
{
  FAIL ("IMPLEMENT DATA treeToVector nodeToData");
  //  auto tree = st_tree::tree<std::tuple<Result, bool>, st_tree::keyed<Action> >{};
  //  tree.insert ({ Result::Undefined, true });
  //  tree.root ().insert (0, { Result::Undefined, true });
  //  tree.root ().insert (2, { Result::Undefined, true });
  //  tree.root ().insert (3, { Result::Undefined, true });
  //  tree.root ()[0].insert (0, { Result::Undefined, true });
  //  tree.root ()[0][0].insert (1, { Result::Undefined, true });
  //  auto myVec = treeToVector (tree, std::tuple<uint8_t, Result>{ 255, Result::Undefined }, std::tuple<uint8_t, Result>{ 254, Result::Undefined }, [] (auto const &node) {
  //    //
  //    return std::tuple<uint8_t, Result>{ node.key ().value (), std::get<0> (node.data ()) };
  //  });
  //  REQUIRE (myVec.size () == 24);
  //  auto result = childrenByPath (myVec, { { 0, Result::Undefined }, { 0, Result::Undefined } });
  //  REQUIRE_FALSE (result.empty ());
  //  REQUIRE (result.at (0) == std::tuple<uint8_t, Result>{ 1, Result::Undefined });
}