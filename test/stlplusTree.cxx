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
#include <expected>
#include <iterator>
#include <vector>

using namespace small_memory_tree;
using namespace small_memory_tree::internals;
TEST_CASE ("generateNodes only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = internals::generateNodes<int> (StlplusTreeAdapter{ tree });
  SECTION ("some test")
  {
    REQUIRE (result.size () == 1);
    REQUIRE (result.at (0) == Node<int>{ 0, 0 });
  }
}
// TODO we do not need to run all the tests for both adapters it is enough if we run it for one and than just compare the SmallMemoryTree created from both adapter
TEST_CASE ("generateNodes multiple elements")
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
  auto result = internals::generateNodes<int> (StlplusTreeAdapter{ tree });
  SECTION ("generateNodes")
  {
    REQUIRE (result.size () == 8);
    REQUIRE (result.at (0) == Node<int>{ 0, 2 });
    REQUIRE (result.at (1) == Node<int>{ 1, 4 });
    REQUIRE (result.at (2) == Node<int>{ 2, 6 });
    REQUIRE (result.at (3) == Node<int>{ 3, 6 });
    REQUIRE (result.at (4) == Node<int>{ 4, 6 });
    REQUIRE (result.at (5) == Node<int>{ 5, 6 });
    REQUIRE (result.at (6) == Node<int>{ 6, 7 });
    REQUIRE (result.at (7) == Node<int>{ 7, 7 });
  }
}

TEST_CASE ("smallSmallMemoryTree only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTree = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };

  SECTION ("construct SmallMemoryTree correctly") { REQUIRE (internals::generateNodes<int> (StlplusTreeAdapter{ tree }) == smallMemoryTree.getNodes ()); }
  SECTION ("calcChildrenCount")
  {
    REQUIRE (calcChildrenCount (smallMemoryTree, 0) == 0);
    REQUIRE (calcChildrenCount (smallMemoryTree, 1).error () == "Index out of bounds nodes.size(): '1' index '1'");
  }
  SECTION ("calcChildrenWithFirstChildIndex 0")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 0);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("calcChildrenWithFirstChildIndex 1 out of bounds") { REQUIRE (calcChildrenWithFirstChildIndex (smallMemoryTree, 1).error () == "Index out of bounds nodes.size(): '1' index '1'"); }

  SECTION ("calcChildrenForPath {0}")
  {
    auto result = calcChildrenForPath (smallMemoryTree, { 0 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calcChildrenForPath {1} wrong value") { REQUIRE (calcChildrenForPath (smallMemoryTree, { 1 }).error () == "invalid path. could not find a match for value with index '0'."); }
}
// TODO we do not need to run all the tests for both adapters it is enough if we run it for one and than just compare the SmallMemoryTree created from both adapter
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
  SECTION ("construct SmallMemoryTree correctly") { REQUIRE (internals::generateNodes<int> (StlplusTreeAdapter{ tree }) == smallMemoryTree.getNodes ()); }
  SECTION ("calcChildrenCount 0") { REQUIRE (calcChildrenCount (smallMemoryTree, 0) == 2); }
  SECTION ("calcChildrenCount 1") { REQUIRE (calcChildrenCount (smallMemoryTree, 1) == 2); }
  SECTION ("calcChildrenCount 2") { REQUIRE (calcChildrenCount (smallMemoryTree, 2) == 2); }
  SECTION ("calcChildrenCount 3") { REQUIRE (calcChildrenCount (smallMemoryTree, 3) == 0); }
  SECTION ("calcChildrenCount 4") { REQUIRE (calcChildrenCount (smallMemoryTree, 4) == 0); }
  SECTION ("calcChildrenCount 5") { REQUIRE (calcChildrenCount (smallMemoryTree, 5) == 0); }
  SECTION ("calcChildrenCount 6") { REQUIRE (calcChildrenCount (smallMemoryTree, 6) == 1); }
  SECTION ("calcChildrenCount 7") { REQUIRE (calcChildrenCount (smallMemoryTree, 7) == 0); }
  SECTION ("calcChildrenWithFirstChildIndex 0")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 0);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 1, 4 }, { 2, 6 } });
  }
  SECTION ("calcChildrenWithFirstChildIndex 1")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 1);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 3, 6 }, { 4, 6 } });
  }
  SECTION ("calcChildrenWithFirstChildIndex 2")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 2);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 5, 6 }, { 6, 7 } });
  }
  SECTION ("calcChildrenWithFirstChildIndex 3")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 3);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("calcChildrenWithFirstChildIndex 4")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 4);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("calcChildrenWithFirstChildIndex 5")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 5);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("calcChildrenWithFirstChildIndex 6")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 6);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 7, 7 } });
  }
  SECTION ("calcChildrenWithFirstChildIndex 7")
  {
    auto result = calcChildrenWithFirstChildIndex (smallMemoryTree, 7);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
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
