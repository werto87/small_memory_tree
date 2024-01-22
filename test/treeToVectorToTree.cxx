/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/smallMemoryTree.hxx"

#include <catch2/catch.hpp>
#include <cstdint>
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
  auto smt = SmallMemoryTree<std::tuple<int, int8_t> >{ tree, { 255, -1 } };
  REQUIRE (smt.generateTreeFromVector () == tree);
}

enum class Result : int
{
  Undefined,
};

class Action
{
public:
  Action () = default;
  explicit Action (std::uint8_t cardPlayed_) : cardPlayed (cardPlayed_) {}
  auto operator<=> (const Action &) const = default;

  [[nodiscard]] std::uint8_t
  value () const
  {
    return cardPlayed;
  };

private:
  std::uint8_t cardPlayed{ 253 };
};

TEST_CASE ("treeToVector")
{
  auto tree = st_tree::tree<std::tuple<Result, bool>, st_tree::keyed<Action> >{};
  tree.insert ({ Result::Undefined, true });
  tree.root ().insert (Action{ 1 }, { Result::Undefined, true });
  tree.root ().insert (Action{ 2 }, { Result::Undefined, true });
  tree.root ().insert (Action{ 3 }, { Result::Undefined, true });
  tree.root ()[Action{ 1 }].insert (Action{ 4 }, { Result::Undefined, true }); // [1] is a key from the parent
  tree.root ()[Action{ 1 }][Action{ 4 }].insert (Action{ 5 }, { Result::Undefined, true });
  auto smt = SmallMemoryTree<std::tuple<uint8_t, Result> >{ tree, std::tuple<uint8_t, Result>{ 255, Result::Undefined }, [] (auto const &node) { return std::tuple<uint8_t, Result>{ node.key ().value (), std::get<0> (node.data ()) }; } };
  REQUIRE (smt.getTreeAsVector ().size () == 20);
  auto result = childrenByPath (smt, { { 253, Result::Undefined } }); // root was not created with a certain value and the default value is 253
  REQUIRE (result.size () == 3);
  REQUIRE (result.at (0) == std::tuple<uint8_t, Result>{ 1, Result::Undefined });
  REQUIRE (result.at (1) == std::tuple<uint8_t, Result>{ 2, Result::Undefined });
  REQUIRE (result.at (2) == std::tuple<uint8_t, Result>{ 3, Result::Undefined });
}