/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/stTree.hxx"
#include "small_memory_tree/stlplusTree.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <vector>

using namespace small_memory_tree;

TEST_CASE ("adapterCheck")
{
  stlplus::ntree<int> nTree{};
  auto root = nTree.insert (0);
  auto rootChild0 = nTree.append (root, 1);
  auto rootChild1 = nTree.append (root, 2);
  nTree.append (rootChild0, 3);
  nTree.append (rootChild0, 4);
  nTree.append (rootChild1, 5);
  auto myChild = nTree.append (rootChild1, 6);
  nTree.append (myChild, 7);
  auto smallMemoryTreeFromStlplusNtree = SmallMemoryTree<int>{ StlplusTreeAdapter{ nTree } };
  auto stTree = st_tree::tree<int>{};
  stTree.insert (0);
  stTree.root ().insert (1);
  stTree.root ().insert (2);
  stTree.root ()[0].insert (3);
  stTree.root ()[0].insert (4);
  stTree.root ()[1].insert (5);
  stTree.root ()[1].insert (6);
  stTree.root ()[1][1].insert (7);
  auto smallMemoryTreeFromStTree = SmallMemoryTree<int>{ StTreeAdapter{ stTree } };
  REQUIRE (smallMemoryTreeFromStlplusNtree == smallMemoryTreeFromStTree);
}