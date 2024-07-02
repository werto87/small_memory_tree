/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/smallMemoryTree.hxx"
#include "small_memory_tree/stlplusTree.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <expected>
#include <iterator>
#include <vector>

using namespace small_memory_tree;
using namespace small_memory_tree::internals;

TEST_CASE ("smallSmallMemoryTree only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTree = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };

  SECTION ("getChildrenCount")
  {
    REQUIRE (getChildrenCount (smallMemoryTree, 0) == 0);
    REQUIRE (getChildrenCount (smallMemoryTree, 1).error () == "Index out of bounds childrenCounts.size(): '1' index '1'");
  }
  SECTION ("childrenBeginAndEndIndex 0")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 0);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) - std::get<1> (result.value ()) == 0);
  }
  SECTION ("childrenBeginAndEndIndex 1 out of bounds") { REQUIRE (childrenBeginAndEndIndex (smallMemoryTree, 1).error () == "Index out of bounds childrenCounts.size(): '1' index '1'"); }

  SECTION ("calcChildrenForPath {0}")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calcChildrenForPath {1} wrong value") { REQUIRE (calcChildrenForPath (smallMemoryTree, { 1 }).error () == "invalid path. could not find a match for value with index '0'."); }
}

TEST_CASE ("smallSmallMemoryTree multiple elements")
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
  auto smallMemoryTree = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  SECTION ("getChildrenCount 0") { REQUIRE (getChildrenCount (smallMemoryTree, 0) == 2); }
  SECTION ("getChildrenCount 1") { REQUIRE (getChildrenCount (smallMemoryTree, 1) == 2); }
  SECTION ("getChildrenCount 2") { REQUIRE (getChildrenCount (smallMemoryTree, 2) == 2); }
  SECTION ("getChildrenCount 3") { REQUIRE (getChildrenCount (smallMemoryTree, 3) == 0); }
  SECTION ("getChildrenCount 4") { REQUIRE (getChildrenCount (smallMemoryTree, 4) == 0); }
  SECTION ("getChildrenCount 5") { REQUIRE (getChildrenCount (smallMemoryTree, 5) == 0); }
  SECTION ("getChildrenCount 6") { REQUIRE (getChildrenCount (smallMemoryTree, 6) == 1); }
  SECTION ("getChildrenCount 7") { REQUIRE (getChildrenCount (smallMemoryTree, 7) == 0); }
  SECTION ("childrenBeginAndEndIndex 0")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 0);
    REQUIRE (result);
    auto values = smallMemoryTree.getValues ();
    REQUIRE (std::vector<int>{ values.at (std::get<0> (result.value ())), values.at (std::get<1> (result.value ()) - 1) } == std::vector<int>{ 1, 2 });
  }
  SECTION ("childrenBeginAndEndIndex 1")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 1);
    REQUIRE (result);
    auto values = smallMemoryTree.getValues ();
    REQUIRE (std::vector<int>{ values.at (std::get<0> (result.value ())), values.at (std::get<1> (result.value ()) - 1) } == std::vector<int>{ 3, 4 });
  }
  SECTION ("childrenBeginAndEndIndex 2")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 2);
    REQUIRE (result);
    auto values = smallMemoryTree.getValues ();
    REQUIRE (std::vector<int>{ values.at (std::get<0> (result.value ())), values.at (std::get<1> (result.value ()) - 1) } == std::vector<int>{ 5, 6 });
  }
  SECTION ("childrenBeginAndEndIndex 3")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 3);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) - std::get<1> (result.value ()) == 0);
  }
  SECTION ("childrenBeginAndEndIndex 4")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 4);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) - std::get<1> (result.value ()) == 0);
  }
  SECTION ("childrenBeginAndEndIndex 5")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 5);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) - std::get<1> (result.value ()) == 0);
  }
  SECTION ("childrenBeginAndEndIndex 6")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 6);
    REQUIRE (result);
    auto values = smallMemoryTree.getValues ();
    REQUIRE (std::vector<int>{ values.at (std::get<0> (result.value ())) } == std::vector<int>{ 7 });
  }
  SECTION ("childrenBeginAndEndIndex 7")
  {
    auto result = childrenBeginAndEndIndex (smallMemoryTree, 7);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) - std::get<1> (result.value ()) == 0);
  }
  SECTION ("calcChildrenForPath 0")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0 });
    REQUIRE (result);
    REQUIRE (result.value () == std::vector<int>{ 1, 2 });
  }
  SECTION ("calcChildrenForPath 0 1")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0, 1 });
    REQUIRE (result);
    REQUIRE (result.value () == std::vector<int>{ 3, 4 });
  }
  SECTION ("calcChildrenForPath 0 1 3")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0, 1, 3 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calcChildrenForPath 0 1 4")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0, 1, 4 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calcChildrenForPath 0 2 5")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 5 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calcChildrenForPath 0 2 6")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 6 });
    REQUIRE (result);
    REQUIRE (result.value () == std::vector<int>{ 7 });
  }
  SECTION ("calcChildrenForPath 0 2 6 7")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 6, 7 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("wrong path")
  {
    SECTION ("calcChildrenForPath empty path")
    {
      auto result = calcChildrenForPath (smallMemoryTree, {});
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "empty path is not allowed");
    }
    SECTION ("calcChildrenForPath 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '0'.");
    }
    SECTION ("calcChildrenForPath 0 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '1'.");
    }
    SECTION ("calcChildrenForPath 0 1 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 1, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calcChildrenForPath 0 1 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 1, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calcChildrenForPath 0 2 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calcChildrenForPath 0 2 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 42 }, true);
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calcChildrenForPath 0 2 6 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 6, 42 }, true);
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '3'.");
    }
    SECTION ("calcChildrenForPath path to long 0 2 6 7 42")
    {
      auto result = calcChildrenForPath (smallMemoryTree, { 0, 2, 6, 7, 42 }, true);
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "Path too long. Last matching index '3'.");
    }
  }
}
