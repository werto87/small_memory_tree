
#include "small_memory_tree/smallMemoryTreeLotsOfChildren.hxx"
#include "small_memory_tree/smallMemoryTree.hxx"
#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

TEST_CASE ("treeData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert ({ 0 });
  auto result = small_memory_tree::internals::treeData (tree);
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == 0);
}

TEST_CASE ("treeData multiple elements")
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
  auto result = small_memory_tree::internals::treeData (tree);
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

TEST_CASE ("treeHierarchy only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::internals::treeHierarchy (tree);
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("treeHierarchy multiple elements")
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
  auto result = small_memory_tree::internals::treeHierarchy (tree);
  REQUIRE (result.size () == 17);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE (" SmallMemoryTreeLotsOfChildrenData multiple elements")
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
  auto result = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  REQUIRE (result.hierarchy.size () == 17);
  REQUIRE (result.data.size () == 8);
  REQUIRE (result.maxChildren == 2);
}

TEST_CASE ("calculateLevelSmallMemoryTreeLotsOfChildrenData and calculateLevels multiple elements")
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
  auto result = small_memory_tree::internals::calculateLevelSmallMemoryTreeLotsOfChildrenData<uint64_t> (small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree));
  auto vec = small_memory_tree::internals::treeToVector (tree, 4444);
  auto levels = small_memory_tree::internals::calculateLevels (vec);
  REQUIRE (result.size () == 5);
  REQUIRE (result.at (0) == levels.at (0).size ());
  REQUIRE (result.at (1) - result.at (0) == levels.at (1).size ());
  REQUIRE (result.at (2) - result.at (1) == levels.at (2).size ());
  REQUIRE (result.at (3) - result.at (2) == levels.at (3).size ());
  REQUIRE (result.at (4) - result.at (3) == levels.at (4).size ());
}

TEST_CASE ("childrenByPath only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("childrenByPath root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
}

TEST_CASE ("childrenByPath multiple elements")
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
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  SECTION ("0")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 1);
    REQUIRE (result->at (1) == 2);
  }
  SECTION ("0 2")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0, 2 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 5);
    REQUIRE (result->at (1) == 6);
  }
  SECTION ("0 2 6")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0, 2, 6 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 1);
    REQUIRE (result->at (0) == 7);
  }
  SECTION ("0 2 6 7")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0, 2, 6, 7 });
    REQUIRE (result.has_value ());
    REQUIRE (result->empty ());
  }
}

TEST_CASE ("treeLevelWithOptionalValues root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  auto result = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, 0);
  REQUIRE (result.at (0).has_value ());
}

TEST_CASE ("treeLevelWithOptionalValues multiple elements")
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
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  SECTION ("level 0")
  {
    auto result = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, 0);
    REQUIRE (result.at (0).has_value ());
    REQUIRE (result.at (0) == 0);
  }
  SECTION ("level 1")
  {
    auto result = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, 1);
    REQUIRE (result.at (0).has_value ());
    REQUIRE (result.at (0) == 1);
    REQUIRE (result.at (1).has_value ());
    REQUIRE (result.at (1) == 2);
  }
  SECTION ("level 2")
  {
    auto result = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, 2);
    REQUIRE (result.at (0).has_value ());
    REQUIRE (result.at (0) == 3);
    REQUIRE (result.at (1).has_value ());
    REQUIRE (result.at (1) == 4);
    REQUIRE (result.at (2).has_value ());
    REQUIRE (result.at (2) == 5);
    REQUIRE (result.at (3).has_value ());
    REQUIRE (result.at (3) == 6);
  }
  SECTION ("level 3")
  {
    auto result = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, 3);
    REQUIRE_FALSE (result.at (0).has_value ());
    REQUIRE_FALSE (result.at (1).has_value ());
    REQUIRE_FALSE (result.at (2).has_value ());
    REQUIRE_FALSE (result.at (3).has_value ());
    REQUIRE_FALSE (result.at (4).has_value ());
    REQUIRE_FALSE (result.at (5).has_value ());
    REQUIRE (result.at (6).has_value ());
    REQUIRE (result.at (6) == 7);
    REQUIRE_FALSE (result.at (7).has_value ());
  }
}

TEST_CASE ("calculateValuesPerLevel only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  auto result = small_memory_tree::internals::calculateValuesPerLevel (smallMemoryTreeLotsOfChildren.getHierarchy (), smallMemoryTreeLotsOfChildren.getLevels ());
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 1);
}

TEST_CASE ("calculateValuesPerLevel multiple elements")
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
  auto smallMemoryTreeLotsOfChildrenData = small_memory_tree::SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
  auto smallMemoryTreeLotsOfChildren = small_memory_tree::SmallMemoryTreeLotsOfChildren<int, uint8_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
  auto result = small_memory_tree::internals::calculateValuesPerLevel (smallMemoryTreeLotsOfChildren.getHierarchy (), smallMemoryTreeLotsOfChildren.getLevels ());
  REQUIRE (result.size () == 5);
  REQUIRE (result.back () == 8);
}