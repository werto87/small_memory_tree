//
// Created by walde on 11/26/22.
//
#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/treeToVector.hxx"
#include <catch2/catch.hpp>
#include <st_tree.h>

using namespace small_memory_tree;

TEST_CASE ("maxChildren", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node->insert (i);
      node = node->insert (i);
    }
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  REQUIRE (maxChildren (myVec, uint8_t{ 255 }) == 2);
}

TEST_CASE ("children", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node->insert (i);
      node = node->insert (i);
    }
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  auto parentIndex = uint64_t{ 9 };
  auto myChildren = children (myVec, parentIndex, uint8_t{ 255 });
  auto offset = myChildren.front ();
  auto numberFromChild = uint64_t{ 1 }; // first child has number one second number 2
  REQUIRE (myVec[offset + parentIndex + numberFromChild] == 1);
}

TEST_CASE ("childWithValue", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node->insert (i);
      node = node->insert (i);
    }
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  auto parentIndex = uint64_t{ 9 };
  auto myChildren = childWithValue (myVec, parentIndex, uint8_t{ 1 }, uint8_t{ 255 });
  REQUIRE (myChildren.has_value ());
}

TEST_CASE ("childrenByPath", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node->insert (i);
      node = node->insert (i);
    }
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  auto parentIndex = uint64_t{ 0 };
  auto myChildren = childrenByPath (myVec, {}, uint8_t{ 255 });
  auto numberFromChild = uint64_t{ 1 }; // first child has number one second number 2
  REQUIRE (myVec[myChildren.front () + parentIndex + numberFromChild] == 2);
}

TEST_CASE ("childrenByPath path", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node->insert (i);
      node = node->insert (i);
    }
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  auto parentIndex = uint64_t{ 3 };
  auto myChildren = childrenByPath (myVec, { 2 }, uint8_t{ 255 });
  auto numberFromChild = uint64_t{ 2 }; // first child has number one second number 2
  REQUIRE (myVec[myChildren.at (numberFromChild - 1) + parentIndex + numberFromChild] == 0);
}

TEST_CASE ("childrenByPath path with 2 values", "[abc]")
{
  auto tree = st_tree::tree<uint8_t>{};
  tree.insert (1);
  auto node = tree.root ().insert (2);
  for (uint8_t i = 0; i < 130; ++i)
    {
      node->insert (i);
      ++i;
      node = node->insert (i);
    }
  auto myVec = treeToVector (tree, uint8_t{ 255 }, uint8_t{ 254 });
  auto parentIndex = uint64_t{ 9 };
  auto myChildren = childrenByPath (myVec, { 2, 1 }, uint8_t{ 255 });
  auto numberFromChild = uint64_t{ 2 }; // first child has number one second number 2
  REQUIRE (myVec[myChildren.at (numberFromChild - 1) + parentIndex + numberFromChild] == 3);
}

TEST_CASE ("2 children", "[abc]")
{
  // TODO fix tests
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (1337);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  auto myVec = treeToVector (tree, 255, 254);
  auto children1 = childrenByPath (myVec, { 1337, 4 }, 255);
  REQUIRE (children1.at (0) == 42);
}

TEST_CASE ("3 children and tuple", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto myVec = treeToVector (tree, std::tuple<uint8_t, int8_t>{ 255, -1 }, std::tuple<uint8_t, int8_t>{ 254, -1 });
  for (auto &value : childrenByPath (myVec, { { 2, 2 }, { 4, 4 } }, { 255, -1 }))
    {
      REQUIRE (value == std::tuple<uint8_t, int8_t>{ 42, 42 });
    }
}