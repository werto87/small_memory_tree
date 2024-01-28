/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "small_memory_tree/smallMemoryTree.hxx"
#include <catch2/catch.hpp>
#include <iostream>

using namespace small_memory_tree;

TEST_CASE ("rootElement")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  auto rootElement = small_memory_tree::rootElement (smt);
  REQUIRE (rootElement == 0);
}

TEST_CASE ("childrenByPath empty path")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  auto myChildren = childrenByPath (smt, {});
  REQUIRE (myChildren.empty ());
}

TEST_CASE ("childrenByPath root as path")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  auto myChildren = childrenByPath (smt, { 0 });
  REQUIRE (myChildren.size () == 2);
  REQUIRE (myChildren.at (0) == 1);
  REQUIRE (myChildren.at (1) == 2);
}

TEST_CASE ("childrenByPath 3 nodes and sibling has same number")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (3);
  tree.root ()[1].insert (4);
  tree.root ()[1][1].insert (69);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  auto myChildren = childrenByPath (smt, { 0, 2, 4 });
  REQUIRE (myChildren.size () == 1);
  REQUIRE (myChildren.at (0) == 69);
}

TEST_CASE ("childrenByPath path with 2 values")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (10);
  auto node = tree.root ().insert (11);
  for (uint8_t i = 0; i < 10; ++i)
    {
      node->insert (boost::numeric_cast<uint8_t> (((i + 1) * 10) + uint8_t{ 2 }));
      node = node->insert (boost::numeric_cast<uint8_t> (((i + 1) * 10) + uint8_t{ 3 }));
    }
  auto smt = SmallMemoryTree<uint8_t>{ tree, 255 };
  auto myChildren = childrenByPath (smt, { 10, 11 });
  REQUIRE (uint64_t{ myChildren.at (0) } == 12);
  REQUIRE (uint64_t{ myChildren.at (1) } == 13);
}

TEST_CASE ("childrenByPath 2 children")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (5);
  auto smt = SmallMemoryTree<int>{ tree, 255 };
  auto children1 = childrenByPath (smt, { 1, 2, 4 });
  REQUIRE (children1.at (0) == 5);
}

TEST_CASE ("childrenByPath 3 children and tuple")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<uint8_t, int8_t> >{ tree, { 255, -1 } };
  for (auto &value : childrenByPath (smt, { { 2, 2 }, { 4, 4 } }))
    {
      REQUIRE (value == std::tuple<uint8_t, int8_t>{ 42, 42 });
    }
}