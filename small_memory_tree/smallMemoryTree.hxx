#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/binaryFind.hxx>
#include <confu_algorithm/createChainViews.hxx>
#include <cstdint>
#include <expected>
#include <format>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>
namespace small_memory_tree
{

template <typename ValueType, typename ChildrenCountType = uint64_t> struct Node
{
  // clang-format off
    [[nodiscard]]
  auto operator<=> (const Node &) const = default;
  // clang-format on

  ValueType value{};
  ChildrenCountType childrenCount{};
};

namespace internals
{

template <typename T> concept IsNode = requires (T const a)
{
  {
    a.begin ()
  } -> std::forward_iterator;
  {
    a.end ()
  } -> std::forward_iterator;
  { a.size () };
  { a.data () };
};

// TODO This does not check what kind of iterator it is.
// if we find out how to check if it is a breath first iterator or not we can rename this
template <typename T> concept HasIteratorToNode = requires (T a)
{
  { a.root () };
  { ++a.constant_breadth_first_traversal_begin () };
  { ++a.constant_breadth_first_traversal_end () };
  {
    *a.constant_breadth_first_traversal_begin ()
  } -> IsNode;
  {
    *a.constant_breadth_first_traversal_end ()
  } -> IsNode;
};
template <typename ValueType, typename ChildrenCountType = uint64_t, typename ChildrenOffsetEndType = uint64_t>
std::vector<ChildrenOffsetEndType>
calcChildrenOffsetEnds (std::vector<Node<ValueType, ChildrenCountType> > const &nodes)
{
  auto result = std::vector<ChildrenOffsetEndType>{};
  std::transform (nodes.begin (), nodes.end (), std::back_inserter (result), [] (Node<ValueType, ChildrenCountType> const &node) { return node.childrenCount; });
  std::partial_sum (result.begin (), result.end (), result.begin ());
  return result;
}

template <typename ValueType, typename ChildrenCountType = uint64_t, HasIteratorToNode TreeAdapter>
[[nodiscard]] std::vector<Node<ValueType, ChildrenCountType> >
generateNodes (TreeAdapter const &treeAdapter)
{
  std::vector<Node<ValueType, ChildrenCountType> > results{};
  std::transform (treeAdapter.constant_breadth_first_traversal_begin (), treeAdapter.constant_breadth_first_traversal_end (), std::back_inserter (results), [] (auto const &node) mutable { return Node<ValueType, ChildrenCountType>{ node.data (), boost::numeric_cast<ChildrenCountType> (std::distance (node.begin (), node.end ())) }; });
  return results;
}

}

template <typename ValueType, typename ChildrenCountType = uint64_t, typename ChildrenOffsetEndType = uint64_t> class SmallMemoryTree
{
public:
  SmallMemoryTree () = default;
  template <internals::HasIteratorToNode TreeAdapter> SmallMemoryTree (TreeAdapter const &treeAdapter) : nodes{ internals::generateNodes<ValueType, ChildrenCountType> (treeAdapter) }, childrenOffsetEnds{ internals::calcChildrenOffsetEnds (nodes) } {}

  SmallMemoryTree (std::vector<Node<ValueType, ChildrenCountType> > nodes_) : nodes{ std::move (nodes_) } {}

  [[nodiscard]] std::vector<Node<ValueType, ChildrenCountType> > const &
  getNodes () const
  {
    return nodes;
  }
  [[nodiscard]] std::vector<ChildrenOffsetEndType> const &
  getChildrenOffsetEnds () const
  {
    return childrenOffsetEnds;
  }

private:
  std::vector<Node<ValueType, ChildrenCountType> > nodes{};
  std::vector<ChildrenOffsetEndType> childrenOffsetEnds{};
};

namespace internals
{
template <typename ValueType, typename ChildrenCountType = uint64_t>
[[nodiscard]] std::expected<ChildrenCountType, std::string>
getChildrenCount (SmallMemoryTree<ValueType, ChildrenCountType> const &SmallMemoryTree, uint64_t index)
{
  auto const &nodes = SmallMemoryTree.getNodes ();
  if (index >= nodes.size ()) return std::unexpected (std::format ("Index out of bounds nodes.size(): '{}' index '{}'", nodes.size (), index));
  return nodes.at (index).childrenCount;
}

template <typename ValueType, typename ChildrenCountType = uint64_t>
[[nodiscard]] std::expected<std::tuple<std::vector<Node<ValueType, ChildrenCountType> >, ChildrenCountType>, std::string>
calcChildrenWithFirstChildIndex (SmallMemoryTree<ValueType, ChildrenCountType> const &SmallMemoryTree, uint64_t index)
{
  if (auto const &childrenCountExpected = getChildrenCount (SmallMemoryTree, index))
    {
      auto const &nodes = SmallMemoryTree.getNodes ();
      auto const &childrenOffsetEnds = SmallMemoryTree.getChildrenOffsetEnds ();
      auto const &childrenCount = boost::numeric_cast<int64_t> (childrenCountExpected.value ());
      auto const &childrenOffsetEnd = nodes.begin () + boost::numeric_cast<int64_t> (childrenOffsetEnds.at (index)) + 1 /*end has to be one element after the last element*/;
      auto const &childrenBegin = childrenOffsetEnd - childrenCount;
      return std::make_tuple (std::vector<Node<ValueType, ChildrenCountType> > (childrenBegin, childrenOffsetEnd), std::distance (nodes.begin (), childrenBegin));
    }
  else
    {
      return std::unexpected (childrenCountExpected.error ());
    }
}
}
template <typename ValueType, typename ChildrenCountType = uint64_t, typename ChildrenOffsetEndType = uint64_t>
[[nodiscard]] std::expected<std::vector<ValueType>, std::string>
calcChildrenForPath (SmallMemoryTree<ValueType, ChildrenCountType> const &SmallMemoryTree, std::vector<ValueType> const &path, bool sortedNodes = false)
{
  if (not path.empty ())
    {
      auto const &nodes = SmallMemoryTree.getNodes ();
      auto childrenWithIndexOfFirstChild = std::tuple (std::vector<Node<ValueType, ChildrenCountType> >{ nodes.front () }, ChildrenOffsetEndType{});
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto &[nodesToCheck, indexOfFirstChild] = childrenWithIndexOfFirstChild;
          auto const &valueToLookFor = path.at (i);
          if (not nodesToCheck.empty ())
            {
              auto nodeItr = nodesToCheck.begin ();
              if (sortedNodes)
                {
                  nodeItr = confu_algorithm::binaryFind (nodesToCheck.begin (), nodesToCheck.end (), valueToLookFor, {}, &Node<ValueType, ChildrenCountType>::value);
                }
              else
                {
                  nodeItr = std::ranges::find (nodesToCheck, valueToLookFor, &Node<ValueType, ChildrenCountType>::value);
                }
              if (nodeItr != nodesToCheck.end ())
                {
                  auto children = internals::calcChildrenWithFirstChildIndex (SmallMemoryTree, indexOfFirstChild + boost::numeric_cast<ChildrenCountType> (std::distance (nodesToCheck.begin (), nodeItr)));
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
      std::ranges::transform (children, std::back_inserter (result), [] (Node<ValueType, ChildrenCountType> const &node) { return node.value; });
      return result;
    }
  else
    {
      return std::unexpected ("empty path is not allowed");
    }
}

}