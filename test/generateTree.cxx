/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "small_memory_tree/smallMemoryTree.hxx"
#include <catch2/catch.hpp>

using namespace small_memory_tree;

TEST_CASE ("generateTree root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree root two children")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree 3 nodes and sibling has same number")
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
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree depth 10")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (10);
  auto node = tree.root ().insert (11);
  for (uint8_t i = 0; i < 10; ++i)
    {
      node->insert (boost::numeric_cast<uint8_t> (((i + 1) * 10) + uint8_t{ 2 }));
      node = node->insert (boost::numeric_cast<uint8_t> (((i + 1) * 10) + uint8_t{ 3 }));
    }
  auto smt = SmallMemoryTree<uint8_t>{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree 3 children and tuple")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<uint8_t, int8_t> >{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree 3 children and tuple crash")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<uint8_t, int8_t> >{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree only root get children of root")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<uint8_t, int8_t> >{ tree };
  REQUIRE (tree == smt.generateTree ());
}

TEST_CASE ("generateTree only root get children of root wrong path")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<uint8_t, int8_t> >{ tree };
  REQUIRE (tree == smt.generateTree ());
}
