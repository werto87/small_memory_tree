//
// Created by walde on 1/22/24.
//

#include "small_memory_tree/smallMemoryTree.hxx"
#include <catch2/catch.hpp>

using namespace small_memory_tree;

TEST_CASE ("test")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  SmallMemoryTree<int>{ tree, 255 };
  FAIL ("THIS TESTS NOTHING");
}
