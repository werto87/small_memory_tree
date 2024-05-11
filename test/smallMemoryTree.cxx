/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/smallMemoryTree.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <st_tree.h>
#include <stdexcept>
#include <stlplus/containers/ntree.hpp>
#include <stlplus/strings/print_basic.hpp>
#include <stlplus/strings/print_int.hpp>
#include <stlplus/strings/print_ntree.hpp>
#include <stlplus/strings/print_string.hpp>
#include <string>
#include <type_traits>
#include <vector>

TEST_CASE ("st_tree treeData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::internals::treeData (tree);
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == 0);
}

TEST_CASE ("st_tree treeData multiple elements")
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

TEST_CASE ("st_tree treeHierarchy only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::internals::treeHierarchy (tree, small_memory_tree::internals::calculateMaxChildren (tree));
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("st_tree treeHierarchy multiple elements")
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

TEST_CASE ("st_tree treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE ("st_tree  SmallMemoryTreeData multiple elements")
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

TEST_CASE ("st_tree childrenByPath only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("st_tree childrenByPath root only")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
}

TEST_CASE ("st_tree childrenByPath multiple elements")
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

TEST_CASE ("st_tree calculateValuesPerLevel only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::calculateValuesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 1);
}

TEST_CASE ("st_tree calculateValuesPerLevel multiple elements")
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

TEST_CASE ("st_tree childrenWithOptionalValues only root")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 0, 0);
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 0);
}

TEST_CASE ("st_tree childrenWithOptionalValues multiple elements")
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

TEST_CASE ("st_tree levelWithOptionalValues")
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

// template <typename T> concept IsNode = requires (T a)
// {
//   {
//     a.begin ()
//   } -> std::forward_iterator;
//   {
//     a.end ()
//   } -> std::forward_iterator;
//   { a.size () };
//   { a.data () };
// };

// // TODO This does not check what kind of iterator it is.
// // if we find out how to check if it is a breath first iterator or not we can rename this
// template <typename T> concept HasIteratorToNode = requires (T a)
// {
//   { ++a.bf_begin () };
//   { ++a.bf_end () };
//   {
//     *a.bf_begin ()
//   } -> IsNode;
//   {
//     *a.bf_end ()
//   } -> IsNode;
// };

// TODO write an adaptor iterator for ntree

template <typename ValueType> struct MyNode
{
  MyNode () = default;
  explicit MyNode (auto node) : _data{ std::move (node->m_data) }
  {
    std::vector<MyNode<int> > myNodes{};
    for (auto const &child : node->m_children)
      {
        childrenValues.emplace_back (child->m_data);
      }
  }
  auto
  begin () const
  {
    return childrenValues.begin ();
  }
  auto
  end () const
  {
    return childrenValues.end ();
  }
  size_t
  size () const
  {
    return childrenValues.size ();
  }

  ValueType const &
  data () const
  {
    return _data;
  }

private:
  ValueType _data{};
  std::vector<ValueType> childrenValues{};
};

template <typename ValueType> struct StlplusTreeWrapper
{
  StlplusTreeWrapper (stlplus::ntree<ValueType> const &tree)
  {
    std::ranges::transform (tree.breadth_first_traversal (), std::back_inserter (myNodes), [] (auto node) { return MyNode<int>{ std::move (node.node ()) }; });
  }

  auto
  root () const
  {
    if (myNodes.empty ()) throw std::logic_error{ "empty tree has no root" };
    return myNodes.front ();
  }
  // TODO should be named cbf_begin() and cbf_end or better constant_breadth_first_traversal_begin() and constant_breadth_first_traversal_end(). Problem is why call it bf_begin then if it is always const. Maybe cbf_begin but than we cant use the library with st_tree out of the box so we have to write this wrapper also for st_tree :(
  auto
  bf_begin () const
  {
    return myNodes.begin ();
  }

  auto
  bf_end () const
  {
    return myNodes.end ();
  }

private:
  std::vector<MyNode<ValueType> > myNodes{};
};

TEST_CASE ("stlplus_tree treeData only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto test = StlplusTreeWrapper{ tree };
  auto result = small_memory_tree::internals::treeData (test);
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == 0);
}

TEST_CASE ("stlplus_tree treeData multiple elements")
{
  FAIL ("impl plx");
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

TEST_CASE ("stlplus_tree treeHierarchy only root")
{
  FAIL ("impl plx");
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::internals::treeHierarchy (tree, small_memory_tree::internals::calculateMaxChildren (tree));
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("stlplus_tree treeHierarchy multiple elements")
{
  FAIL ("impl plx");
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

TEST_CASE ("stlplus_tree treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  FAIL ("impl plx");
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto result = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE ("stlplus_tree  SmallMemoryTreeData multiple elements")
{
  FAIL ("impl plx");
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

TEST_CASE ("stlplus_tree childrenByPath only root")
{
  FAIL ("impl plx");
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("stlplus_tree childrenByPath root only")
{
  FAIL ("impl plx");
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
}

TEST_CASE ("stlplus_tree childrenByPath multiple elements")
{
  FAIL ("impl plx");
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

TEST_CASE ("stlplus_tree calculateValuesPerLevel only root")
{
  FAIL ("impl plx");
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::calculateValuesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 1);
}

TEST_CASE ("stlplus_tree calculateValuesPerLevel multiple elements")
{
  FAIL ("impl plx");
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

TEST_CASE ("stlplus_tree childrenWithOptionalValues only root")
{
  FAIL ("impl plx");
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  auto smallMemoryTreeData = small_memory_tree::SmallMemoryTreeData<int, uint8_t> (tree);
  auto smallMemoryTree = small_memory_tree::SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, 0, 0);
  REQUIRE (result.size () == 1);
  REQUIRE (result.front () == 0);
}

TEST_CASE ("stlplus_tree childrenWithOptionalValues multiple elements")
{
  FAIL ("impl plx");
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

TEST_CASE ("stlplus_tree levelWithOptionalValues")
{
  FAIL ("impl plx");
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
