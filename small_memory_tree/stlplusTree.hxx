#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "smallMemoryTree.hxx"
#include "smallMemoryTreeAdapter.hxx"
#include <stlplus/containers/ntree.hpp>
#include <stlplus/strings/print_basic.hpp>
#include <stlplus/strings/print_int.hpp>
#include <stlplus/strings/print_ntree.hpp>
#include <stlplus/strings/print_string.hpp>
#include <type_traits>
namespace small_memory_tree
{

template <typename ValueType, typename NodeType> struct StlplusNodeAdapter : public BaseNodeAdapter<ValueType, NodeType>
{

  StlplusNodeAdapter (NodeType const &node) : BaseNodeAdapter<ValueType, NodeType>{ generateNodeData (node), generateChildrenData (node) } {}

  ValueType
  generateNodeData (NodeType const &node) override
  {
    return node.m_data;
  };

  std::vector<ValueType>
  generateChildrenData (NodeType const &node) override
  {
    auto results = std::vector<ValueType>{};
    std::ranges::transform (node.m_children, std::back_inserter (results), [] (auto const &childNode) { return childNode->m_data; });
    return results;
  };
};

template <typename ValueType, typename NodeType = stlplus::ntree_node<ValueType>, typename TreeType = stlplus::ntree<ValueType> > struct StlplusTreeAdapter : public BaseTreeAdapter<StlplusNodeAdapter, ValueType, NodeType, TreeType>
{
  StlplusTreeAdapter (stlplus::ntree<ValueType> const &tree) : BaseTreeAdapter<StlplusNodeAdapter, ValueType, NodeType, TreeType>{ generateNodeAdapters (tree) } {}
  std::vector<StlplusNodeAdapter<ValueType, NodeType> >
  generateNodeAdapters (TreeType const &tree) override
  {
    auto results = std::vector<StlplusNodeAdapter<ValueType, NodeType> >{};
    std::ranges::transform (tree.breadth_first_traversal (), std::back_inserter (results), [] (auto const &nodeWrapper) { return StlplusNodeAdapter<ValueType, NodeType>{ *nodeWrapper.node () }; });
    return results;
  }
};

template <typename ValueType, typename MaxChildrenType, typename LevelType, typename ValuesPerLevelType>
inline stlplus::ntree<ValueType>
generateStlplusTree (SmallMemoryTree<ValueType, MaxChildrenType, LevelType, ValuesPerLevelType> const &smallMemoryTree)
{
  // TODO implement this
  auto const &data = smallMemoryTree.getData ();
  auto result = stlplus::ntree<ValueType>{};
  auto parentNode = result.insert (data.front ());
  if (data.size () == 1) // only one element which means tree with only a root node
    {
      return result;
    }
  else
    {
      throw "implement";
    }
  return result;
}
}

namespace stlplus
{
template <typename ValueType>
bool
operator== (ntree<ValueType> const &lhs, ntree<ValueType> const &rhs)
{
  if (lhs.size () != rhs.size ())
    {
      return false;
    }
  else
    {
      auto rhsItr = rhs.postfix_begin ();
      for (auto lhsItr = lhs.postfix_begin (); lhsItr != lhs.postfix_end (); lhsItr++, rhsItr++)
        {
          if (lhsItr.simplify ().node ()->m_data != rhsItr.simplify ().node ()->m_data)
            {
              return false;
            }
        }
    }
  return true;
}

template <typename ValueType>
bool
operator!= (ntree<ValueType> const &lhs, ntree<ValueType> const &rhs)
{
  return !(lhs == rhs);
}
}