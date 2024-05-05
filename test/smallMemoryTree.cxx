#include "small_memory_tree/smallMemoryTree.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <vector>

TEST_CASE ("treeData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
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
  auto result = small_memory_tree::internals::treeHierarchy (tree, small_memory_tree::internals::calculateMaxChildren (tree));
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
  auto result = small_memory_tree::internals::treeHierarchy (tree, small_memory_tree::internals::calculateMaxChildren (tree));
  REQUIRE (result.size () == 17);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE (" SmallMemoryTreeData multiple elements")
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
  auto result = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  REQUIRE (result.hierarchy.size () == 17);
  REQUIRE (result.data.size () == 8);
  REQUIRE (result.maxChildren == 2);
}

TEST_CASE ("childrenByPath only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("childrenByPath root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
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
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 1);
    REQUIRE (result->at (1) == 2);
  }
  SECTION ("0 2")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 5);
    REQUIRE (result->at (1) == 6);
  }
  SECTION ("0 2 6")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 1);
    REQUIRE (result->at (0) == 7);
  }
  SECTION ("0 2 6 7")
  {
    auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6, 7 });
    REQUIRE (result.has_value ());
    REQUIRE (result->empty ());
  }
}

TEST_CASE ("calculateValuesPerLevel only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::calculateValuesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
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
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::calculateValuesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 5);
  REQUIRE (result.back () == 8);
}

TEST_CASE ("treeLevelWithOptionalValues only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 0, 0);
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 0);
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
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0 0")
  {
    auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 0, 0);
    REQUIRE (result.size () == 1);
    REQUIRE (result.at (0) == 0);
  }
  SECTION ("1 0")
  {
    auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 1, 0);
    REQUIRE (result.size () == 2);
    REQUIRE (result.at (0) == 1);
    REQUIRE (result.at (1) == 2);
  }
  SECTION ("2 1")
  {
    auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 2, 1);
    REQUIRE (result.size () == 2);
    REQUIRE (result.at (0) == 5);
    REQUIRE (result.at (1) == 6);
  }
  SECTION ("3 0")
  {
    auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 3, 0);
    REQUIRE (result.size () == 2);
    REQUIRE_FALSE (result.at (0).has_value ());
    REQUIRE_FALSE (result.at (1).has_value ());
  }
  SECTION ("3 1")
  {
    auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 3, 3);
    REQUIRE (result.size () == 2);
    REQUIRE (result.at (0) == 7);
    REQUIRE_FALSE (result.at (1).has_value ());
  }
  SECTION ("4 0")
  {
    auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 4, 0);
    REQUIRE (result.size () == 2);
    REQUIRE_FALSE (result.at (0).has_value ());
    REQUIRE_FALSE (result.at (1).has_value ());
  }
  SECTION ("4 1 node value to high") { REQUIRE_THROWS (small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 4, 1)); }
  SECTION ("5 0") { REQUIRE_THROWS (small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 5, 0)); }
}

TEST_CASE ("my test")
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
  auto result = small_memory_tree::SmallMemoryTree<int> (tree);
  using small_memory_tree::internals::levelWithOptionalValues;
  REQUIRE (levelWithOptionalValues (result, 0).size () == 1);
  REQUIRE (levelWithOptionalValues (result, 1).size () == 2);
  REQUIRE (levelWithOptionalValues (result, 2).size () == 4);
  REQUIRE (levelWithOptionalValues (result, 3).size () == 8);
  REQUIRE (levelWithOptionalValues (result, 4).size () == 2);
}