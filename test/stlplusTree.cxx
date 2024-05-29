/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/stlplusTree.hxx"
#include "small_memory_tree/smallMemoryTree.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <iterator>
#include <vector>

using namespace small_memory_tree;

TEST_CASE ("stlplus_tree treeData only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = internals::treeData (StlplusTreeAdapter{ tree });
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == 0);
}

TEST_CASE ("stlplus_tree treeData multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  tree.append (myChild, 7);
  auto result = internals::treeData (StlplusTreeAdapter{ tree });
  REQUIRE (result.size () == 8);
  REQUIRE (result.at (0) == 0);
  REQUIRE (result.at (1) == 1);
  REQUIRE (result.at (2) == 2);
  REQUIRE (result.at (3) == 3);
  REQUIRE (result.at (4) == 4);
  REQUIRE (result.at (5) == 5);
  REQUIRE (result.at (6) == 6);
  REQUIRE (result.at (7) == 7);
}

TEST_CASE ("stlplus_tree treeHierarchy only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = internals::treeHierarchy (StlplusTreeAdapter{ tree }, internals::calculateMaxChildren (StlplusTreeAdapter{ tree }));
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("stlplus_tree treeHierarchy multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  tree.append (myChild, 7);
  auto result = internals::treeHierarchy (StlplusTreeAdapter{ tree }, internals::calculateMaxChildren (StlplusTreeAdapter{ tree }));
  REQUIRE (result.size () == 17);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("stlplus_tree treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE ("stlplus_tree  SmallMemoryTreeData multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  tree.append (myChild, 7);
  auto result = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE (result.hierarchy.size () == 17);
  REQUIRE (result.data.size () == 8);
  REQUIRE (result.maxChildren == 2);
}

TEST_CASE ("stlplus_tree childrenByPath only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("stlplus_tree childrenByPath root only")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
}

TEST_CASE ("stlplus_tree childrenByPath multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  auto newChild = tree.append (myChild, 7);
  tree.append (newChild, 8);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 1);
    REQUIRE (result->at (1) == 2);
  }
  SECTION ("0 2")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 5);
    REQUIRE (result->at (1) == 6);
  }
  SECTION ("0 2 6")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 1);
    REQUIRE (result->at (0) == 7);
  }
  SECTION ("0 2 6 7")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6, 7 });
    REQUIRE (result->size () == 1);
    REQUIRE (result->at (0) == 8);
  }
  SECTION ("0 2 6 7 8")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6, 7, 8 });
    REQUIRE (result.has_value ());
    REQUIRE (result->empty ());
  }
}

TEST_CASE ("stlplus_tree calculateNodeIndexesPerLevel only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::calculateNodeIndexesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.front ().size () == 1);
  REQUIRE (result.front ().front () == 0);
}

TEST_CASE ("stlplus_tree calculateNodeIndexesPerLevel  root with 2 children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::calculateNodeIndexesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.front ().size () == 1);
  REQUIRE (result.front ().front () == 0);
  REQUIRE (result.at (1).size () == 2);
  REQUIRE (result.at (1).at (0) == 0);
  REQUIRE (result.at (1).at (1) == 1);
}

TEST_CASE ("stlplus_tree calculateNodeIndexesPerLevel multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  tree.append (myChild, 7);
  auto smallMemoryTreeData = SmallMemoryTreeData<int> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int>{ smallMemoryTreeData };
  auto result = internals::calculateNodeIndexesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 5);
  REQUIRE (result.at (0).size () == 1);
  REQUIRE (result.at (0).at (0) == 0);
  REQUIRE (result.at (1).size () == 2);
  REQUIRE (result.at (1).at (0) == 0);
  REQUIRE (result.at (1).at (1) == 1);
  REQUIRE (result.at (2).size () == 4);
  REQUIRE (result.at (2).at (0) == 0);
  REQUIRE (result.at (2).at (1) == 1);
  REQUIRE (result.at (2).at (2) == 2);
  REQUIRE (result.at (2).at (3) == 3);
  REQUIRE (result.at (3).size () == 1);
  REQUIRE (result.at (3).at (0) == 6);
  REQUIRE (result.at (4).size () == 0);
}

TEST_CASE ("stlplus_tree getTotalValuesUsedUntilLevel root with 2 children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto result = SmallMemoryTree<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE (result.getTotalValuesUsedUntilLevel (0) == 0);
  REQUIRE (result.getTotalValuesUsedUntilLevel (1) == 1);
  REQUIRE (result.getTotalValuesUsedUntilLevel (2) == 3);
}

TEST_CASE ("stlplus_tree getTotalValuesUsedUntilLevel root with 2 children out of bound")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto result = SmallMemoryTree<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE_THROWS (result.getTotalValuesUsedUntilLevel (3));
}

TEST_CASE ("stlplus_tree childrenAndUsedValuesUntilChildren only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 0);
  REQUIRE (std::get<0> (result).empty ());
}

TEST_CASE ("stlplus_tree childrenAndUsedValuesUntilChildren multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  auto myNewChild = tree.append (myChild, 7);
  tree.append (myNewChild, 8);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 0);
    REQUIRE (std::get<0> (result).size () == 2);
    REQUIRE (std::get<0> (result).at (0) == 1);
    REQUIRE (std::get<0> (result).at (1) == 2);
  }
  SECTION ("0 1") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 1)); }
  SECTION ("1 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 1, 0);
    REQUIRE (std::get<0> (result).size () == 2);
    REQUIRE (std::get<0> (result).at (0) == 3);
    REQUIRE (std::get<0> (result).at (1) == 4);
  }
  SECTION ("1 1")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 1, 1);
    REQUIRE (std::get<0> (result).size () == 2);
    REQUIRE (std::get<0> (result).at (0) == 5);
    REQUIRE (std::get<0> (result).at (1) == 6);
  }
  SECTION ("1 2") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 1, 2)); }
  SECTION ("2 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 0);
    REQUIRE (std::get<0> (result).empty ());
  }

  SECTION ("2 1")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 1);
    REQUIRE (std::get<0> (result).empty ());
  }
  SECTION ("2 2")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 2);
    REQUIRE (std::get<0> (result).empty ());
  }
  SECTION ("2 3")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 3);
    REQUIRE (std::get<0> (result).size () == 1);
    REQUIRE (std::get<0> (result).at (0) == 7);
  }
  SECTION ("2 4") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 4)); }

  SECTION ("3 0") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 0)); }
  SECTION ("3 1") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 1)); }
  SECTION ("3 2") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 2)); }
  SECTION ("3 3") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 3)); }
  SECTION ("3 4") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 4)); }
  SECTION ("3 5") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 5)); }
  SECTION ("3 6")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 6);
    REQUIRE (std::get<0> (result).size () == 1);
  }
  SECTION ("3 7") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 7)); }
  SECTION ("3 8") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 8)); }

  SECTION ("4 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 4, 0);
    REQUIRE (std::get<0> (result).empty ());
  }
  SECTION ("4 1") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 4, 1)); }
}

TEST_CASE ("stlplus_tree levelWithOptionalValues")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  tree.append (myChild, 7);
  auto result = SmallMemoryTree<int> (StlplusTreeAdapter{ tree });
  using internals::levelWithOptionalValues;
  REQUIRE (levelWithOptionalValues (result, 0).size () == 1);
  REQUIRE (levelWithOptionalValues (result, 1).size () == 2);
  REQUIRE (levelWithOptionalValues (result, 2).size () == 4);
  REQUIRE (levelWithOptionalValues (result, 3).size () == 8);
  REQUIRE (levelWithOptionalValues (result, 4).size () == 2);
}

TEST_CASE ("stlplus tree == and != test same trees")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  tree.append (myChild, 7);
  stlplus::ntree<int> newTree{};
  auto root1 = newTree.insert (0);
  auto rootChild01 = newTree.append (root1, 1);
  auto rootChild11 = newTree.append (root1, 2);
  newTree.append (rootChild01, 3);
  newTree.append (rootChild01, 4);
  newTree.append (rootChild11, 5);
  auto myChild1 = newTree.append (rootChild11, 6);
  newTree.append (myChild1, 7);
  REQUIRE (tree == newTree);
  REQUIRE_FALSE (tree != newTree);
}

TEST_CASE ("stlplus tree == and != different trees")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 42);
  tree.append (myChild, 7);
  stlplus::ntree<int> newTree{};
  auto root1 = newTree.insert (0);
  auto rootChild01 = newTree.append (root1, 1);
  auto rootChild11 = newTree.append (root1, 2);
  newTree.append (rootChild01, 3);
  newTree.append (rootChild01, 4);
  newTree.append (rootChild11, 5);
  auto myChild1 = newTree.append (rootChild11, 6);
  newTree.append (myChild1, 7);
  REQUIRE_FALSE (tree == newTree);
}

TEST_CASE ("stlplus generateTree root only")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smt = SmallMemoryTree<int> (StlplusTreeAdapter{ tree });
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree root two children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree root three children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  tree.append (root, 1);
  tree.append (root, 2);
  tree.append (root, 3);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree root one child max child 2")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  tree.append (rootChild0, 2);
  tree.append (rootChild0, 3);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree 4 levels and sibling has same number")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 3);
  auto rootChild1Child = tree.append (rootChild1, 4);
  tree.append (rootChild1Child, 69);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree depth 10")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (10);
  auto node = tree.append (root, 11);
  for (int i = 0; i < 10; ++i)
    {
      tree.append (node, boost::numeric_cast<int> (((i + 1) * 10) + int{ 2 }));
      node = tree.append (node, boost::numeric_cast<int> (((i + 1) * 10) + int{ 3 }));
    }
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree only root")
{
  stlplus::ntree<std::tuple<int, int> > tree{};
  auto root = tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree 3 children and tuple")
{
  stlplus::ntree<std::tuple<int, int> > tree{};
  auto root = tree.insert ({ 1, 1 });
  auto child0 = tree.append (root, { 2, 2 });
  tree.append (root, { 3, 3 });
  tree.append (root, { 69, 69 });
  auto child0child = tree.append (child0, { 4, 4 });
  tree.append (child0child, { 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("partitionPoint")
{
  SECTION ("true, false")
  {
    auto data = std::vector{ true, false };
    REQUIRE (std::distance (data.cbegin (), dataEndForChildren (data.cbegin (), data.cend ())) == 1);
  }
  SECTION ("true, true, false")
  {
    auto data = std::vector{ true, true, false };
    REQUIRE (std::distance (data.cbegin (), dataEndForChildren (data.cbegin (), data.cend ())) == 2);
  }
  SECTION ("false, false")
  {
    auto data = std::vector{ false, false };
    REQUIRE (std::distance (data.cbegin (), dataEndForChildren (data.cbegin (), data.cend ())) == 0);
  }
  SECTION ("true, true")
  {
    auto data = std::vector{ true, true };
    REQUIRE (std::distance (data.cbegin (), dataEndForChildren (data.cbegin (), data.cend ())) == 2);
  }
}
