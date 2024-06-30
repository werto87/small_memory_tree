#include "small_memory_tree/stTree.hxx"
#include <catch2/catch.hpp>
using namespace small_memory_tree;
TEST_CASE ("generateStTree")
{
  auto stTree = st_tree::tree<int>{};
  stTree.insert (0);
  stTree.root ().insert (1);
  stTree.root ().insert (2);
  stTree.root ()[0].insert (3);
  stTree.root ()[0].insert (4);
  stTree.root ()[1].insert (5);
  stTree.root ()[1].insert (6);
  stTree.root ()[1][1].insert (7);
  auto smallMemoryTree = SmallMemoryTree<int>{ StTreeAdapter{ stTree } };
  auto testSmallMemoryTree = SmallMemoryTree<int>{ StTreeAdapter{ generateStTree (smallMemoryTree).value () } };
  REQUIRE (testSmallMemoryTree == smallMemoryTree);
}