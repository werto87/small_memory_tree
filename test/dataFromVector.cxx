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

TEST_CASE ("offsetOfChildrenByPath", "[abc]")
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
  auto myChildren = offsetOfChildrenByPath (myVec, {}, uint8_t{ 255 });
  auto numberFromChild = uint64_t{ 1 }; // first child has number one second number 2
  REQUIRE (myVec[myChildren.front () + parentIndex + numberFromChild] == 2);
}

TEST_CASE ("offsetOfChildrenByPath path", "[abc]")
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
  auto myChildren = offsetOfChildrenByPath (myVec, { 2 }, uint8_t{ 255 });
  auto numberFromChild = uint64_t{ 2 }; // first child has number one second number 2
  REQUIRE (myVec[myChildren.at (numberFromChild - 1) + parentIndex + numberFromChild] == 0);
}

TEST_CASE ("offsetOfChildrenByPath path with 2 values", "[abc]")
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
  auto myChildren = offsetOfChildrenByPath (myVec, { 2, 1 }, uint8_t{ 255 });
  auto numberFromChild = uint64_t{ 2 }; // first child has number one second number 2
  REQUIRE (myVec[myChildren.at (numberFromChild - 1) + parentIndex + numberFromChild] == 3);
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
  REQUIRE (myChildren.at (0) == 2);
  REQUIRE (myChildren.at (1) == 3);
}