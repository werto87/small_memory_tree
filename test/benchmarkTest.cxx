/*
 Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
 Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
  */

#include "small_memory_tree/smallMemoryTree.hxx"
#include "small_memory_tree/smallMemoryTreeLotsOfChildren.hxx"
#include <catch2/catch.hpp>
#include <cstdint>
#include <iostream>

using namespace small_memory_tree;

TEST_CASE ("childrenByPath", "[!benchmark]")
{
  SECTION ("few elements")
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
    SECTION ("childrenByPath 0 2 6 7")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<int, uint8_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<int, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<int>{ 0, 2, 6, 7 }); };
      auto smt = SmallMemoryTree<int>{ tree, 255 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<int>{ 0, 2, 6, 7 }); };
    }
  }
  SECTION ("max children == 100")
  {
    auto tree = st_tree::tree<uint64_t>{};
    tree.insert (0);
    for (auto i = uint64_t{}; i < 100; ++i)
      {
        tree.root ().insert (i);
      }
    SECTION ("childrenByPath 0 1")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t, uint8_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<uint64_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<uint64_t>{ 0, 1 }); };
      auto smt = SmallMemoryTree<uint64_t>{ tree, 255 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<uint64_t>{ 0, 1 }); };
    }
    SECTION ("childrenByPath 0 99")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t, uint8_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<uint64_t, uint8_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<uint64_t>{ 0, 99 }); };
      auto smt = SmallMemoryTree<uint64_t>{ tree, 255 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<uint64_t>{ 0, 99 }); };
    }
  }
  SECTION ("max children == 1000")
  {
    auto tree = st_tree::tree<uint64_t>{};
    tree.insert (0);
    for (auto i = uint64_t{}; i < 1000; ++i)
      {
        tree.root ().insert (i);
      }
    SECTION ("childrenByPath 0 0")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t, uint64_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<uint64_t, uint64_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<uint64_t>{ 0, 0 }); };
      auto smt = SmallMemoryTree<uint64_t>{ tree, 5555555 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<uint64_t>{ 0, 0 }); };
    }
    SECTION ("childrenByPath 0 999")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t, uint64_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<uint64_t, uint64_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<uint64_t>{ 0, 999 }); };
      auto smt = SmallMemoryTree<uint64_t>{ tree, 5555555 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<uint64_t>{ 0, 999 }); };
    }
  }
  SECTION ("max children == 10000")
  {
    auto tree = st_tree::tree<uint64_t>{};
    tree.insert (0);
    for (auto i = uint64_t{}; i < 10000; ++i)
      {
        tree.root ().insert (i);
      }
    SECTION ("childrenByPath 0 0")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t, uint64_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<uint64_t, uint64_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<uint64_t>{ 0, 0 }); };
      auto smt = SmallMemoryTree<uint64_t>{ tree, 5555555 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<uint64_t>{ 0, 0 }); };
    }
    SECTION ("childrenByPath 0 9999")
    {
      auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t, uint64_t> (tree);
      auto smallMemoryTreeLotsOfChildren = SmallMemoryTreeLotsOfChildren<uint64_t, uint64_t>{ smallMemoryTreeLotsOfChildrenData };
      BENCHMARK ("SmallMemoryTreeLotsOfChildren") { return childrenByPath (smallMemoryTreeLotsOfChildren, std::vector<uint64_t>{ 0, 9999 }); };
      auto smt = SmallMemoryTree<uint64_t>{ tree, 5555555 };
      BENCHMARK ("SmallMemoryTree childrenByPath") { return childrenByPath (smt, std::vector<uint64_t>{ 0, 9999 }); };
    }
  }
}

TEST_CASE ("memory consumption benchmark test", "[memory]")
{
  // SECTION ("few elements")
  // {
  //   auto tree = st_tree::tree<uint64_t>{};
  //   tree.insert (0);
  //   tree.root ().insert (1);
  //   tree.root ().insert (2);
  //   tree.root ()[0].insert (3);
  //   tree.root ()[0].insert (4);
  //   tree.root ()[1].insert (5);
  //   tree.root ()[1].insert (6);
  //   tree.root ()[1][1].insert (7);
  //   auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t> (tree);
  // }
  // SECTION ("max children == 100")
  // {
  //   auto tree = st_tree::tree<uint64_t>{};
  //   tree.insert (0);
  //   for (auto i = uint64_t{}; i < 100; ++i)
  //     {
  //       tree.root ().insert (i);
  //     }
  //   auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t> (tree);
  // }
  // SECTION ("max children == 1000")
  // {
  //   auto tree = st_tree::tree<uint64_t>{};
  //   tree.insert (0);
  //   for (auto i = uint64_t{}; i < 1000; ++i)
  //     {
  //       tree.root ().insert (i);
  //     }
  //   auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t> (tree);
  // }
  SECTION ("max children == 100000 just the tree")
  {
    auto tree = st_tree::tree<uint64_t>{}; // 160KiB
    tree.insert (0);
    for (auto i = uint64_t{}; i < 1000; ++i)
      {
        tree.root ().insert (i);
      }
    auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t> (tree);
    // auto vec = std::vector<bool> (1000000);
  }
  // SECTION ("max children == 10000")
  // {
  //   auto tree = st_tree::tree<uint64_t>{};
  //   tree.insert (0);
  //   for (auto i = uint64_t{}; i < 10000; ++i)
  //     {
  //       tree.root ().insert (i);
  //     }
  //   auto smallMemoryTreeLotsOfChildrenData = SmallMemoryTreeLotsOfChildrenData<uint64_t> (tree);
  // }
  // SECTION ("1'000'000 byte") { auto vec = std::vector<uint8_t> (1000000); }
}
