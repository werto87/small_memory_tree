#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/apiError.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/binaryFind.hxx>
#include <expected>
#include <format>
#include <numeric>
namespace small_memory_tree
{

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

}

template <typename ValueType, typename ChildrenOffsetEndType = uint64_t> class SmallMemoryTree
{
public:
  SmallMemoryTree () = default;
  template <internals::HasIteratorToNode TreeAdapter> SmallMemoryTree (TreeAdapter const &treeAdapter)
  {
    auto childrenSum = ChildrenOffsetEndType{};
    std::for_each (treeAdapter.constant_breadth_first_traversal_begin (), treeAdapter.constant_breadth_first_traversal_end (), [&] (auto const &node) mutable {
      values.push_back (node.data ());
      childrenSum += boost::numeric_cast<ChildrenOffsetEndType> (std::distance (node.begin (), node.end ()));
      childrenOffsetEnds.push_back (childrenSum);
    });
    values.shrink_to_fit ();
    childrenOffsetEnds.shrink_to_fit ();
  }

  SmallMemoryTree (std::vector<ValueType> values_, std::vector<ChildrenOffsetEndType> childrenOffsetEnds_) : values{ std::move (values_) }, childrenOffsetEnds{ std::move (childrenOffsetEnds_) } {}

  // clang-format off
    [[nodiscard]]
  auto operator<=> (const SmallMemoryTree &) const = default;
  // clang-format on

  [[nodiscard]] std::vector<ValueType> const &
  getValues () const
  {
    return values;
  }

  [[nodiscard]] std::vector<ChildrenOffsetEndType> const &
  getChildrenOffsetEnds () const
  {
    return childrenOffsetEnds;
  }

private:
  std::vector<ValueType> values{};
  std::vector<ChildrenOffsetEndType> childrenOffsetEnds{};
};

namespace internals
{
template <typename ValueType, typename ChildrenOffsetEndType = uint64_t>
[[nodiscard]] std::expected<ChildrenOffsetEndType, std::error_condition>
getChildrenCount (SmallMemoryTree<ValueType, ChildrenOffsetEndType> const &smallMemoryTree, uint64_t index)
{
  auto const &childrenOffsetEnds = smallMemoryTree.getChildrenOffsetEnds ();
  if (index >= childrenOffsetEnds.size ()) return std::unexpected (make_error_condition (ApiError::OutOfRange));
  return (index == 0) ? childrenOffsetEnds.at (index) : childrenOffsetEnds.at (index) - childrenOffsetEnds.at (index - 1);
}

template <typename ValueType, typename ChildrenOffsetEndType = uint64_t>
[[nodiscard]] std::expected<std::tuple<uint64_t, uint64_t>, std::error_condition>
childrenBeginAndEndIndex (SmallMemoryTree<ValueType, ChildrenOffsetEndType> const &smallMemoryTree, uint64_t index)
{
  if (auto const &childrenCountExpected = getChildrenCount (smallMemoryTree, index))
    {
      auto const &childrenOffsetEnds = smallMemoryTree.getChildrenOffsetEnds ();
      auto const &childrenCount = boost::numeric_cast<uint64_t> (childrenCountExpected.value ());
      auto const &childrenOffsetEnd = boost::numeric_cast<uint64_t> (childrenOffsetEnds.at (index)) + 1 /*end has to be one element after the last element*/;
      auto const &childrenBegin = childrenOffsetEnd - childrenCount;
      return std::tuple<uint64_t, uint64_t>{ childrenBegin, childrenOffsetEnd };
    }
  else
    {
      return std::unexpected (childrenCountExpected.error ());
    }
}
}
template <typename ValueType, typename ChildrenOffsetEndType = uint64_t>
[[nodiscard]] std::expected<std::vector<ValueType>, std::error_condition>
calcChildrenForPath (SmallMemoryTree<ValueType, ChildrenOffsetEndType> const &smallMemoryTree, std::vector<ValueType> const &path, bool sortedNodes = false)
{
  if (not path.empty ())
    {
      auto const &values = smallMemoryTree.getValues ();
      auto nodesToCheckBeginAndEndIndex = std::optional<std::tuple<int64_t, int64_t> >{ std::make_tuple (0, 1) };
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto const &valueToLookFor = path.at (i);
          if (nodesToCheckBeginAndEndIndex.has_value ())
            {
              auto const &childrenBegin = values.begin () + std::get<0> (nodesToCheckBeginAndEndIndex.value ());
              auto const &childrenEnd = values.begin () + std::get<1> (nodesToCheckBeginAndEndIndex.value ());
              auto const &children = std::span{ childrenBegin, childrenEnd };
              auto nodeItr = children.begin ();
              if (sortedNodes)
                {
                  nodeItr = confu_algorithm::binaryFind (children.begin (), children.end (), valueToLookFor);
                }
              else
                {
                  nodeItr = std::ranges::find (children, valueToLookFor);
                }
              if (nodeItr != children.end ())
                {
                  if (auto const &childrenBeginAndEndIndexExpected = internals::childrenBeginAndEndIndex (smallMemoryTree, boost::numeric_cast<uint64_t> (std::get<0> (nodesToCheckBeginAndEndIndex.value ()) + std::distance (children.begin (), nodeItr))))
                    {
                      auto const &childrenCount = std::get<1> (childrenBeginAndEndIndexExpected.value ()) - std::get<0> (childrenBeginAndEndIndexExpected.value ());
                      if (childrenCount != 0)
                        {
                          nodesToCheckBeginAndEndIndex = childrenBeginAndEndIndexExpected.value ();
                        }
                      else
                        {
                          nodesToCheckBeginAndEndIndex = std::nullopt;
                        }
                    }
                  else
                    {
                      return std::unexpected (childrenBeginAndEndIndexExpected.error ());
                    }
                }
              else
                {
                  return std::unexpected (make_error_condition (ApiError::PathDoesNotMatch));
                }
            }
          else
            {
              return std::unexpected (make_error_condition (ApiError::PathTooLong));
            }
        }
      if (nodesToCheckBeginAndEndIndex.has_value ())
        {
          return std::vector<ValueType> (values.begin () + std::get<0> (nodesToCheckBeginAndEndIndex.value ()), values.begin () + std::get<1> (nodesToCheckBeginAndEndIndex.value ()));
        }
      else
        {
          return std::vector<ValueType>{};
        }
    }
  else
    {
      return std::unexpected (make_error_condition (ApiError::EmptyPath));
    }
}
}