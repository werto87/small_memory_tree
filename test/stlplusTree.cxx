#include "small_memory_tree/stlplusTree.hxx"
#include <catch2/catch.hpp>
using namespace small_memory_tree;
TEST_CASE ("generateStlplusTree")
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
  auto smallMemoryTree = SmallMemoryTree<int>{ StlplusTreeAdapter{ nTree } };
  auto testSmallMemoryTree = SmallMemoryTree<int>{ StlplusTreeAdapter{ generateStlplusTree (smallMemoryTree).value () } };
  REQUIRE (testSmallMemoryTree == smallMemoryTree);
}