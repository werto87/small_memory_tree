/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/stTree.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <string>
#include <vector>

using namespace small_memory_tree;

TEST_CASE ("st_tree treeData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = internals::treeData (StTreeAdapter{ tree });
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == 0);
}

TEST_CASE ("st_tree treeData multiple elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto result = internals::treeData (StTreeAdapter{ tree });
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

TEST_CASE ("st_tree treeHierarchy only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = internals::treeHierarchy (StTreeAdapter{ tree }, internals::calculateMaxChildren (StTreeAdapter{ tree }));
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("st_tree treeHierarchy multiple elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto result = internals::treeHierarchy (StTreeAdapter{ tree }, internals::calculateMaxChildren (StTreeAdapter{ tree }));
  REQUIRE (result.size () == 17);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("st_tree treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE ("st_tree  SmallMemoryTreeData multiple elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto result = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  REQUIRE (result.hierarchy.size () == 17);
  REQUIRE (result.data.size () == 8);
  REQUIRE (result.maxChildren == 2);
}

TEST_CASE ("st_tree childrenByPath only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("st_tree childrenByPath root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
}

TEST_CASE ("st_tree childrenByPath multiple elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
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
    REQUIRE (result.has_value ());
    REQUIRE (result->empty ());
  }
}

TEST_CASE ("st_tree calculateValuesPerLevel only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::calculateValuesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 1);
}

TEST_CASE ("st_tree calculateValuesPerLevel multiple elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::calculateValuesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 5);
  REQUIRE (result.back () == 8);
}

TEST_CASE ("st_tree childrenWithOptionalValues only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::childrenWithOptionalValues (smallMemoryTree, 0, 0);
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 0);
}

TEST_CASE ("st_tree childrenWithOptionalValues multiple elements")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0 0")
  {
    auto result = internals::childrenWithOptionalValues (smallMemoryTree, 0, 0);
    REQUIRE (result.size () == 1);
    REQUIRE (result.at (0) == 0);
  }
  SECTION ("1 0")
  {
    auto result = internals::childrenWithOptionalValues (smallMemoryTree, 1, 0);
    REQUIRE (result.size () == 2);
    REQUIRE (result.at (0) == 1);
    REQUIRE (result.at (1) == 2);
  }
  SECTION ("2 1")
  {
    auto result = internals::childrenWithOptionalValues (smallMemoryTree, 2, 1);
    REQUIRE (result.size () == 2);
    REQUIRE (result.at (0) == 5);
    REQUIRE (result.at (1) == 6);
  }
  SECTION ("3 0")
  {
    auto result = internals::childrenWithOptionalValues (smallMemoryTree, 3, 0);
    REQUIRE (result.size () == 2);
    REQUIRE_FALSE (result.at (0).has_value ());
    REQUIRE_FALSE (result.at (1).has_value ());
  }
  SECTION ("3 1")
  {
    auto result = internals::childrenWithOptionalValues (smallMemoryTree, 3, 3);
    REQUIRE (result.size () == 2);
    REQUIRE (result.at (0) == 7);
    REQUIRE_FALSE (result.at (1).has_value ());
  }
  SECTION ("4 0")
  {
    auto result = internals::childrenWithOptionalValues (smallMemoryTree, 4, 0);
    REQUIRE (result.size () == 2);
    REQUIRE_FALSE (result.at (0).has_value ());
    REQUIRE_FALSE (result.at (1).has_value ());
  }
  SECTION ("4 1 node value to high") { REQUIRE_THROWS (internals::childrenWithOptionalValues (smallMemoryTree, 4, 1)); }
  SECTION ("5 0") { REQUIRE_THROWS (internals::childrenWithOptionalValues (smallMemoryTree, 5, 0)); }
}

TEST_CASE ("st_tree levelWithOptionalValues")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  tree.root ()[1][1].insert (7);
  auto result = SmallMemoryTree<int> (StTreeAdapter{ tree });
  using internals::levelWithOptionalValues;
  REQUIRE (levelWithOptionalValues (result, 0).size () == 1);
  REQUIRE (levelWithOptionalValues (result, 1).size () == 2);
  REQUIRE (levelWithOptionalValues (result, 2).size () == 4);
  REQUIRE (levelWithOptionalValues (result, 3).size () == 8);
  REQUIRE (levelWithOptionalValues (result, 4).size () == 2);
}

TEST_CASE ("generateTree root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree root two children")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree root three children")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree root one child max child 2")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ()[0].insert (2);
  tree.root ()[0].insert (3);
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
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
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
  auto generatedTree = generateStTree (smt);
  auto smtFromGeneratedTree = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
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
  auto smt = SmallMemoryTree<int>{ StTreeAdapter{ tree } };
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
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StTreeAdapter{ tree } };
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
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StTreeAdapter{ tree } };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree only root get children of root")
{
  auto tree = st_tree::tree<std::tuple<int, int> >{};
  tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StTreeAdapter{ tree } };
  REQUIRE (tree == generateStTree (smt));
}

TEST_CASE ("generateTree only root get children of root wrong path")
{
  auto tree = st_tree::tree<std::tuple<int, int> >{};
  tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StTreeAdapter{ tree } };
  REQUIRE (tree == generateStTree (smt));
}
