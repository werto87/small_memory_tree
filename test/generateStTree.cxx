/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "small_memory_tree/generateStTree.hxx"
#include "small_memory_tree/smallMemoryTree.hxx"
#include <catch2/catch.hpp>

using namespace small_memory_tree;

TEST_CASE ("generateTree root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree root two children")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree root three children")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree root one child max child 2")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ()[0].insert (2);
  tree.root ()[0].insert (3);
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree 4 levels and sibling has same number")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (3);
  tree.root ()[1].insert (4);
  tree.root ()[1][0].insert (69);
  auto smt = SmallMemoryTree<int>{ tree };
  auto generatedTree = generateStTree (smt);
  auto smtFromGeneratedTree = SmallMemoryTree<int>{ tree };
  REQUIRE (smt == smtFromGeneratedTree);
}

TEST_CASE ("generateTree depth 10")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (10);
  auto node = tree.root ().insert (11);
  for (int i = 0; i < 10; ++i)
    {
      node->insert (boost::numeric_cast<int> (((i + 1) * 10) + int{ 2 }));
      node = node->insert (boost::numeric_cast<int> (((i + 1) * 10) + int{ 3 }));
    }
  auto smt = SmallMemoryTree<int>{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree 3 children and tuple")
{
  auto tree = st_tree::tree<std::tuple<int, int> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree 3 children and tuple crash")
{
  auto tree = st_tree::tree<std::tuple<int, int> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree only root get children of root")
{
  auto tree = st_tree::tree<std::tuple<int, int> >{};
  tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ tree };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree only root get children of root wrong path")
{
  auto tree = st_tree::tree<std::tuple<int, int> >{};
  tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ tree };
  REQUIRE (tree == generateStTree (smt));
}
