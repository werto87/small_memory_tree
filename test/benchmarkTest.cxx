/*
 Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
 Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
  */

#include "small_memory_tree/stTree.hxx"
#include <catch2/catch.hpp>
#include <cstdint>

using namespace small_memory_tree;

TEST_CASE ("SmallMemoryTree calcChildrenForPath", "[!benchmark]")
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
    SECTION ("calcChildrenForPath 0 2 6 7")
    {
      auto smallMemoryTree = SmallMemoryTree<int, uint8_t>{ StTreeAdapter{ tree } };
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<int>{ 0, 2, 6, 7 }); };
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
    SECTION ("calcChildrenForPath 0 1")
    {
      auto smallMemoryTree = SmallMemoryTree<uint64_t, uint8_t>{ StTreeAdapter{ tree } };
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 1 }); };
    }
    SECTION ("calcChildrenForPath 0 99")
    {
      auto smallMemoryTree = SmallMemoryTree<uint64_t, uint8_t>{ StTreeAdapter{ tree } };
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 99 }); };
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
    auto smallMemoryTree = SmallMemoryTree<uint64_t, uint64_t>{ StTreeAdapter{ tree } };
    SECTION ("calcChildrenForPath 0 0")
    {
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 0 }); };
    }
    SECTION ("calcChildrenForPath 0 999")
    {
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 999 }); };
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
    auto smallMemoryTree = SmallMemoryTree<uint64_t, uint64_t>{ StTreeAdapter{ tree } };
    SECTION ("calcChildrenForPath 0 0")
    {
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 0 }); };
    }
    SECTION ("calcChildrenForPath 0 9999")
    {
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 9999 }); };
    }
    SECTION ("calcChildrenForPath 0 9999 with binary find")
    {
      BENCHMARK ("SmallMemoryTree") { return calcChildrenForPath (smallMemoryTree, std::vector<uint64_t>{ 0, 9999 }, true); };
    }
  }
}