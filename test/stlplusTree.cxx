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

template <typename ValueType, typename ChildrenOffsetEnd = uint64_t> struct Node
{
  // clang-format off
    [[nodiscard]]
  auto operator<=> (const Node &) const = default;
  // clang-format on

  ValueType value{};
  ChildrenOffsetEnd childrenOffsetEnd{};
};

namespace small_memory_tree::internals
{
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t, HasIteratorToNode TreeAdapter>
[[nodiscard]] std::vector<Node<ValueType, ChildrenOffsetEnd> >
generateNodes (TreeAdapter const &treeAdapter)
{
  std::vector<Node<ValueType, ChildrenOffsetEnd> > results{};
  auto childrenOffsetEnd = uint64_t{};
  std::transform (treeAdapter.constant_breadth_first_traversal_begin (), treeAdapter.constant_breadth_first_traversal_end (), std::back_inserter (results), [&childrenOffsetEnd] (auto const &node) mutable {
    childrenOffsetEnd = childrenOffsetEnd + boost::numeric_cast<uint64_t> (std::distance (node.begin (), node.end ()));
    return Node<ValueType, ChildrenOffsetEnd>{ node.data (), boost::numeric_cast<uint64_t> (childrenOffsetEnd) };
  });
  return results;
}
}
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t> class SmallMemoryTreeNew
{
public:
  SmallMemoryTreeNew () = default;
  template <internals::HasIteratorToNode TreeAdapter> SmallMemoryTreeNew (TreeAdapter const &treeAdapter) : nodes{ small_memory_tree::internals::generateNodes<ValueType> (treeAdapter) } {}

  SmallMemoryTreeNew (std::vector<Node<ValueType, ChildrenOffsetEnd> > nodes_) : nodes{ std::move (nodes_) } {}

  [[nodiscard]] std::vector<Node<ValueType, ChildrenOffsetEnd> > const &
  getNodes () const
  {
    return nodes;
  }

private:
  std::vector<Node<ValueType, ChildrenOffsetEnd> > nodes{};
};

template <typename ValueType, typename ChildrenOffsetEnd = uint64_t>
[[nodiscard]] std::expected<ChildrenOffsetEnd, std::string>
getChildrenCount (SmallMemoryTreeNew<ValueType, ChildrenOffsetEnd> const &smallMemoryTreeNew, uint64_t index)
{
  auto const &nodes = smallMemoryTreeNew.getNodes ();
  if (index >= nodes.size ()) return std::unexpected (std::format ("Index out of bounds nodes.size(): '{}' index '{}'", nodes.size (), index));
  return (index == 0) ? nodes.at (index).childrenOffsetEnd : nodes.at (index).childrenOffsetEnd - nodes.at (index - 1).childrenOffsetEnd;
}

template <typename ValueType, typename ChildrenOffsetEnd = uint64_t>
[[nodiscard]] std::expected<std::tuple<std::vector<Node<ValueType, ChildrenOffsetEnd> >, ChildrenOffsetEnd>, std::string>
getChildrenWithFirstChildIndex (SmallMemoryTreeNew<ValueType, ChildrenOffsetEnd> const &smallMemoryTreeNew, uint64_t index)
{
  if (auto const &childrenCountExpected = getChildrenCount (smallMemoryTreeNew, index))
    {
      auto const &nodes = smallMemoryTreeNew.getNodes ();
      auto const &childrenCount = boost::numeric_cast<int64_t> (childrenCountExpected.value ());
      auto const &childrenOffsetEnd = nodes.begin () + boost::numeric_cast<int64_t> (nodes.at (index).childrenOffsetEnd) + 1 /*end has to be one element after the last element*/;
      auto const &childrenBegin = childrenOffsetEnd - childrenCount;
      return std::make_tuple (std::vector<Node<ValueType, ChildrenOffsetEnd> > (childrenBegin, childrenOffsetEnd), std::distance (nodes.begin (), childrenBegin));
    }
  else
    {
      return std::unexpected (childrenCountExpected.error ());
    }
}

template <typename ValueType, typename ChildrenOffsetEnd = uint64_t>
[[nodiscard]] std::expected<std::vector<ValueType>, std::string>
calculateChildrenForPath (SmallMemoryTreeNew<ValueType, ChildrenOffsetEnd> const &smallMemoryTreeNew, std::vector<ValueType> const &path, bool sortedNodes = false)
{
  if (not path.empty ())
    {
      auto const &nodes = smallMemoryTreeNew.getNodes ();
      auto childrenWithIndexOfFirstChild = std::tuple (std::vector<Node<ValueType, ChildrenOffsetEnd> >{ nodes.front () }, ChildrenOffsetEnd{});
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto &[nodesToCheck, indexOfFirstChild] = childrenWithIndexOfFirstChild;
          auto const &valueToLookFor = path.at (i);
          if (not nodesToCheck.empty ())
            {
              auto nodeItr = nodesToCheck.begin ();
              if (sortedNodes)
                {
                  nodeItr = confu_algorithm::binaryFind (nodesToCheck.begin (), nodesToCheck.end (), valueToLookFor, {}, &Node<ValueType, ChildrenOffsetEnd>::value);
                }
              else
                {
                  nodeItr = std::ranges::find (nodesToCheck, valueToLookFor, &Node<ValueType, ChildrenOffsetEnd>::value);
                }
              if (nodeItr != nodesToCheck.end ())
                {
                  auto children = getChildrenWithFirstChildIndex (smallMemoryTreeNew, indexOfFirstChild + boost::numeric_cast<ChildrenOffsetEnd> (std::distance (nodesToCheck.begin (), nodeItr)));
                  if (children)
                    {
                      childrenWithIndexOfFirstChild = std::move (children.value ());
                    }
                  else
                    {
                      return std::unexpected (children.error ());
                    }
                }
              else
                {
                  return std::unexpected (std::format ("invalid path. could not find a match for value with index '{}'.", i));
                }
            }
          else
            {
              return std::unexpected (std::format ("Path too long. Last matching index '{}'.", i - 1));
            }
        }
      auto const &children = std::get<0> (childrenWithIndexOfFirstChild);
      auto result = std::vector<ValueType>{};
      std::ranges::transform (children, std::back_inserter (result), [] (Node<ValueType, ChildrenOffsetEnd> const &node) { return node.value; });
      return result;
    }
  else
    {
      return std::unexpected ("empty path is not allowed");
    }
  return std::vector<ValueType>{};
}

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
// TODO we do not need to run all the tests for both adapters it is enough if we run it for one and than just compare the SmallMemoryTreeNew created from both adapter
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

TEST_CASE ("smallSmallMemoryTreeNew only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTree = SmallMemoryTreeNew<int>{ StlplusTreeAdapter{ tree } };

  SECTION ("construct SmallMemoryTreeNew correctly") { REQUIRE (internals::generateNodes<int> (StlplusTreeAdapter{ tree }) == smallMemoryTree.getNodes ()); }
  SECTION ("getChildrenCount")
  {
    REQUIRE (getChildrenCount (smallMemoryTree, 0) == 0);
    REQUIRE (getChildrenCount (smallMemoryTree, 1).error () == "Index out of bounds nodes.size(): '1' index '1'");
  }
  SECTION ("getChildrenWithFirstChildIndex 0")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 0);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("getChildrenWithFirstChildIndex 1 out of bounds") { REQUIRE (getChildrenWithFirstChildIndex (smallMemoryTree, 1).error () == "Index out of bounds nodes.size(): '1' index '1'"); }

  SECTION ("calculateChildrenForPath {0}")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calculateChildrenForPath {1} wrong value") { REQUIRE (calculateChildrenForPath (smallMemoryTree, { 1 }).error () == "invalid path. could not find a match for value with index '0'."); }
}
// TODO we do not need to run all the tests for both adapters it is enough if we run it for one and than just compare the SmallMemoryTreeNew created from both adapter
TEST_CASE ("smallSmallMemoryTreeNew multiple elements")
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
  auto smallMemoryTree = SmallMemoryTreeNew<int>{ StlplusTreeAdapter{ tree } };
  SECTION ("construct SmallMemoryTreeNew correctly") { REQUIRE (internals::generateNodes<int> (StlplusTreeAdapter{ tree }) == smallMemoryTree.getNodes ()); }
  SECTION ("getChildrenCount 0") { REQUIRE (getChildrenCount (smallMemoryTree, 0) == 2); }
  SECTION ("getChildrenCount 1") { REQUIRE (getChildrenCount (smallMemoryTree, 1) == 2); }
  SECTION ("getChildrenCount 2") { REQUIRE (getChildrenCount (smallMemoryTree, 2) == 2); }
  SECTION ("getChildrenCount 3") { REQUIRE (getChildrenCount (smallMemoryTree, 3) == 0); }
  SECTION ("getChildrenCount 4") { REQUIRE (getChildrenCount (smallMemoryTree, 4) == 0); }
  SECTION ("getChildrenCount 5") { REQUIRE (getChildrenCount (smallMemoryTree, 5) == 0); }
  SECTION ("getChildrenCount 6") { REQUIRE (getChildrenCount (smallMemoryTree, 6) == 1); }
  SECTION ("getChildrenCount 7") { REQUIRE (getChildrenCount (smallMemoryTree, 7) == 0); }
  SECTION ("getChildrenWithFirstChildIndex 0")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 0);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 1, 4 }, { 2, 6 } });
  }
  SECTION ("getChildrenWithFirstChildIndex 1")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 1);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 3, 6 }, { 4, 6 } });
  }
  SECTION ("getChildrenWithFirstChildIndex 2")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 2);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 5, 6 }, { 6, 7 } });
  }
  SECTION ("getChildrenWithFirstChildIndex 3")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 3);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("getChildrenWithFirstChildIndex 4")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 4);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("getChildrenWithFirstChildIndex 5")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 5);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("getChildrenWithFirstChildIndex 6")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 6);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()) == std::vector<Node<int> >{ { 7, 7 } });
  }
  SECTION ("getChildrenWithFirstChildIndex 7")
  {
    auto result = getChildrenWithFirstChildIndex (smallMemoryTree, 7);
    REQUIRE (result);
    REQUIRE (std::get<0> (result.value ()).empty ());
  }
  SECTION ("calculateChildrenForPath 0")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0 });
    REQUIRE (result);
    REQUIRE (result.value () == std::vector<int>{ 1, 2 });
  }
  SECTION ("calculateChildrenForPath 0 1")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0, 1 });
    REQUIRE (result);
    REQUIRE (result.value () == std::vector<int>{ 3, 4 });
  }
  SECTION ("calculateChildrenForPath 0 1 3")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0, 1, 3 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calculateChildrenForPath 0 1 4")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0, 1, 4 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calculateChildrenForPath 0 2 5")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 5 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("calculateChildrenForPath 0 2 6")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 6 });
    REQUIRE (result);
    REQUIRE (result.value () == std::vector<int>{ 7 });
  }
  SECTION ("calculateChildrenForPath 0 2 6 7")
  {
    auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 6, 7 });
    REQUIRE (result);
    REQUIRE (result->empty ());
  }
  SECTION ("wrong path")
  {
    SECTION ("calculateChildrenForPath empty path")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, {});
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "empty path is not allowed");
    }
    SECTION ("calculateChildrenForPath 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '0'.");
    }
    SECTION ("calculateChildrenForPath 0 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '1'.");
    }
    SECTION ("calculateChildrenForPath 0 1 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 1, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calculateChildrenForPath 0 1 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 1, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calculateChildrenForPath 0 2 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 42 });
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calculateChildrenForPath 0 2 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 42 }, true);
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '2'.");
    }
    SECTION ("calculateChildrenForPath 0 2 6 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 6, 42 }, true);
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "invalid path. could not find a match for value with index '3'.");
    }
    SECTION ("calculateChildrenForPath path to long 0 2 6 7 42")
    {
      auto result = calculateChildrenForPath (smallMemoryTree, { 0, 2, 6, 7, 42 }, true);
      REQUIRE_FALSE (result);
      REQUIRE (result.error () == "Path too long. Last matching index '3'.");
    }
  }
}

TEST_CASE ("stlplus_tree treeData only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = internals::treeData (StlplusTreeAdapter{ tree });
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == 0);
}

TEST_CASE ("stlplus_tree treeData multiple elements")
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
  auto result = internals::treeData (StlplusTreeAdapter{ tree });
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
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = internals::treeHierarchy (StlplusTreeAdapter{ tree }, internals::calculateMaxChildren (StlplusTreeAdapter{ tree }));
  REQUIRE (result.size () == 1);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("stlplus_tree treeHierarchy multiple elements")
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
  auto result = internals::treeHierarchy (StlplusTreeAdapter{ tree }, internals::calculateMaxChildren (StlplusTreeAdapter{ tree }));
  REQUIRE (result.size () == 17);
  REQUIRE (result.at (0) == true);
}

TEST_CASE ("stlplus_tree treeToSmallMemoryTreeLotsOfChildrenData only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto result = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE (result.hierarchy.size () == 1);
  REQUIRE (result.data.size () == 1);
  REQUIRE (result.maxChildren == 0);
}

TEST_CASE ("stlplus_tree  SmallMemoryTreeData multiple elements")
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
  auto result = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE (result.hierarchy.size () == 17);
  REQUIRE (result.data.size () == 8);
  REQUIRE (result.maxChildren == 2);
}

TEST_CASE ("stlplus_tree childrenByPath only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
  REQUIRE (result->empty ());
}

TEST_CASE ("stlplus_tree childrenByPath root only")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
  REQUIRE (result.has_value ());
}

TEST_CASE ("stlplus_tree childrenByPath multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  auto newChild = tree.append (myChild, 7);
  tree.append (newChild, 8);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 1);
    REQUIRE (result->at (1) == 2);
  }
  SECTION ("0 2")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 2);
    REQUIRE (result->at (0) == 5);
    REQUIRE (result->at (1) == 6);
  }
  SECTION ("0 2 6")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6 });
    REQUIRE (result.has_value ());
    REQUIRE (result->size () == 1);
    REQUIRE (result->at (0) == 7);
  }
  SECTION ("0 2 6 7")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6, 7 });
    REQUIRE (result->size () == 1);
    REQUIRE (result->at (0) == 8);
  }
  SECTION ("0 2 6 7 8")
  {
    auto result = childrenByPath (smallMemoryTree, std::vector<int>{ 0, 2, 6, 7, 8 });
    REQUIRE (result.has_value ());
    REQUIRE (result->empty ());
  }
}

TEST_CASE ("stlplus_tree calculateNodeIndexesPerLevel only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::calculateNodeIndexesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.front ().size () == 1);
  REQUIRE (result.front ().front () == 0);
}

TEST_CASE ("stlplus_tree calculateNodeIndexesPerLevel  root with 2 children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::calculateNodeIndexesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.front ().size () == 1);
  REQUIRE (result.front ().front () == 0);
  REQUIRE (result.at (1).size () == 2);
  REQUIRE (result.at (1).at (0) == 0);
  REQUIRE (result.at (1).at (1) == 1);
}

TEST_CASE ("stlplus_tree calculateNodeIndexesPerLevel multiple elements")
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
  auto smallMemoryTreeData = SmallMemoryTreeData<int> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int>{ smallMemoryTreeData };
  auto result = internals::calculateNodeIndexesPerLevel (smallMemoryTree.getHierarchy (), smallMemoryTree.getLevels ());
  REQUIRE (result.size () == 5);
  REQUIRE (result.at (0).size () == 1);
  REQUIRE (result.at (0).at (0) == 0);
  REQUIRE (result.at (1).size () == 2);
  REQUIRE (result.at (1).at (0) == 0);
  REQUIRE (result.at (1).at (1) == 1);
  REQUIRE (result.at (2).size () == 4);
  REQUIRE (result.at (2).at (0) == 0);
  REQUIRE (result.at (2).at (1) == 1);
  REQUIRE (result.at (2).at (2) == 2);
  REQUIRE (result.at (2).at (3) == 3);
  REQUIRE (result.at (3).size () == 1);
  REQUIRE (result.at (3).at (0) == 6);
  REQUIRE (result.at (4).size () == 0);
}

TEST_CASE ("stlplus_tree getTotalValuesUsedUntilLevel root with 2 children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto result = SmallMemoryTree<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE (result.getTotalValuesUsedUntilLevel (0) == 0);
  REQUIRE (result.getTotalValuesUsedUntilLevel (1) == 1);
  REQUIRE (result.getTotalValuesUsedUntilLevel (2) == 3);
}

TEST_CASE ("stlplus_tree getTotalValuesUsedUntilLevel root with 2 children out of bound")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto result = SmallMemoryTree<int, uint8_t> (StlplusTreeAdapter{ tree });
  REQUIRE_THROWS (result.getTotalValuesUsedUntilLevel (3));
}

TEST_CASE ("stlplus_tree childrenAndUsedValuesUntilChildren only root")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t, uint8_t>{ smallMemoryTreeData };
  auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 0);
  REQUIRE (std::get<0> (result).empty ());
}

TEST_CASE ("stlplus_tree childrenAndUsedValuesUntilChildren multiple elements")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 6);
  auto myNewChild = tree.append (myChild, 7);
  tree.append (myNewChild, 8);
  auto smallMemoryTreeData = SmallMemoryTreeData<int, uint8_t> (StlplusTreeAdapter{ tree });
  auto smallMemoryTree = SmallMemoryTree<int, uint8_t>{ smallMemoryTreeData };
  SECTION ("0 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 0);
    REQUIRE (std::get<0> (result).size () == 2);
    REQUIRE (std::get<0> (result).at (0) == 1);
    REQUIRE (std::get<0> (result).at (1) == 2);
  }
  SECTION ("0 1") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 1)); }
  SECTION ("1 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 1, 0);
    REQUIRE (std::get<0> (result).size () == 2);
    REQUIRE (std::get<0> (result).at (0) == 3);
    REQUIRE (std::get<0> (result).at (1) == 4);
  }
  SECTION ("1 1")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 1, 1);
    REQUIRE (std::get<0> (result).size () == 2);
    REQUIRE (std::get<0> (result).at (0) == 5);
    REQUIRE (std::get<0> (result).at (1) == 6);
  }
  SECTION ("1 2") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 1, 2)); }
  SECTION ("2 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 0);
    REQUIRE (std::get<0> (result).empty ());
  }

  SECTION ("2 1")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 1);
    REQUIRE (std::get<0> (result).empty ());
  }
  SECTION ("2 2")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 2);
    REQUIRE (std::get<0> (result).empty ());
  }
  SECTION ("2 3")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 3);
    REQUIRE (std::get<0> (result).size () == 1);
    REQUIRE (std::get<0> (result).at (0) == 7);
  }
  SECTION ("2 4") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 2, 4)); }

  SECTION ("3 0") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 0)); }
  SECTION ("3 1") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 1)); }
  SECTION ("3 2") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 2)); }
  SECTION ("3 3") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 3)); }
  SECTION ("3 4") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 4)); }
  SECTION ("3 5") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 5)); }
  SECTION ("3 6")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 6);
    REQUIRE (std::get<0> (result).size () == 1);
  }
  SECTION ("3 7") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 7)); }
  SECTION ("3 8") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 3, 8)); }

  SECTION ("4 0")
  {
    auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 4, 0);
    REQUIRE (std::get<0> (result).empty ());
  }
  SECTION ("4 1") { REQUIRE_THROWS (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 4, 1)); }
}

TEST_CASE ("stlplus_tree levelWithOptionalValues")
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
  auto result = SmallMemoryTree<int> (StlplusTreeAdapter{ tree });
  using internals::levelWithOptionalValues;
  REQUIRE (levelWithOptionalValues (result, 0).size () == 1);
  REQUIRE (levelWithOptionalValues (result, 1).size () == 2);
  REQUIRE (levelWithOptionalValues (result, 2).size () == 4);
  REQUIRE (levelWithOptionalValues (result, 3).size () == 8);
  REQUIRE (levelWithOptionalValues (result, 4).size () == 2);
}

TEST_CASE ("stlplus tree == and != test same trees")
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
  stlplus::ntree<int> newTree{};
  auto root1 = newTree.insert (0);
  auto rootChild01 = newTree.append (root1, 1);
  auto rootChild11 = newTree.append (root1, 2);
  newTree.append (rootChild01, 3);
  newTree.append (rootChild01, 4);
  newTree.append (rootChild11, 5);
  auto myChild1 = newTree.append (rootChild11, 6);
  newTree.append (myChild1, 7);
  REQUIRE (tree == newTree);
  REQUIRE_FALSE (tree != newTree);
}

TEST_CASE ("stlplus tree == and != different trees")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 5);
  auto myChild = tree.append (rootChild1, 42);
  tree.append (myChild, 7);
  stlplus::ntree<int> newTree{};
  auto root1 = newTree.insert (0);
  auto rootChild01 = newTree.append (root1, 1);
  auto rootChild11 = newTree.append (root1, 2);
  newTree.append (rootChild01, 3);
  newTree.append (rootChild01, 4);
  newTree.append (rootChild11, 5);
  auto myChild1 = newTree.append (rootChild11, 6);
  newTree.append (myChild1, 7);
  REQUIRE_FALSE (tree == newTree);
}

TEST_CASE ("stlplus generateTree root only")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto smt = SmallMemoryTree<int> (StlplusTreeAdapter{ tree });
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree root two children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree root three children")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  tree.append (root, 1);
  tree.append (root, 2);
  tree.append (root, 3);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree root one child max child 2")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  tree.append (rootChild0, 2);
  tree.append (rootChild0, 3);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree 4 levels and sibling has same number")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (0);
  auto rootChild0 = tree.append (root, 1);
  auto rootChild1 = tree.append (root, 2);
  tree.append (rootChild0, 3);
  tree.append (rootChild0, 4);
  tree.append (rootChild1, 3);
  auto rootChild1Child = tree.append (rootChild1, 4);
  tree.append (rootChild1Child, 69);
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree depth 10")
{
  stlplus::ntree<int> tree{};
  auto root = tree.insert (10);
  auto node = tree.append (root, 11);
  for (int i = 0; i < 10; ++i)
    {
      tree.append (node, boost::numeric_cast<int> (((i + 1) * 10) + int{ 2 }));
      node = tree.append (node, boost::numeric_cast<int> (((i + 1) * 10) + int{ 3 }));
    }
  auto smt = SmallMemoryTree<int>{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree only root")
{
  stlplus::ntree<std::tuple<int, int> > tree{};
  auto root = tree.insert ({ 1, 1 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("stlplus generateTree 3 children and tuple")
{
  stlplus::ntree<std::tuple<int, int> > tree{};
  auto root = tree.insert ({ 1, 1 });
  auto child0 = tree.append (root, { 2, 2 });
  tree.append (root, { 3, 3 });
  tree.append (root, { 69, 69 });
  auto child0child = tree.append (child0, { 4, 4 });
  tree.append (child0child, { 42, 42 });
  auto smt = SmallMemoryTree<std::tuple<int, int> >{ StlplusTreeAdapter{ tree } };
  REQUIRE (tree == generateStlplusTree (smt));
}

TEST_CASE ("partitionPoint")
{
  SECTION ("true, false")
  {
    auto data = std::vector{ true, false };
    REQUIRE (std::distance (data.cbegin (), internals::dataEndForChildren (data.cbegin (), data.cend ())) == 1);
  }
  SECTION ("true, true, false")
  {
    auto data = std::vector{ true, true, false };
    REQUIRE (std::distance (data.cbegin (), internals::dataEndForChildren (data.cbegin (), data.cend ())) == 2);
  }
  SECTION ("false, false")
  {
    auto data = std::vector{ false, false };
    REQUIRE (std::distance (data.cbegin (), internals::dataEndForChildren (data.cbegin (), data.cend ())) == 0);
  }
  SECTION ("true, true")
  {
    auto data = std::vector{ true, true };
    REQUIRE (std::distance (data.cbegin (), internals::dataEndForChildren (data.cbegin (), data.cend ())) == 2);
  }
}
