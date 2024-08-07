#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "smallMemoryTree.hxx"
#include "smallMemoryTreeAdapter.hxx"
#include <st_tree.h>
namespace small_memory_tree
{

template <typename ValueType, typename NodeType> struct StTreeNodeAdapter : public BaseNodeAdapter<ValueType, NodeType>
{

  StTreeNodeAdapter (NodeType const &node) : BaseNodeAdapter<ValueType, NodeType>{ generateNodeData (node), generateChildrenData (node) } {}

  ValueType
  generateNodeData (NodeType const &node)
  {
    return node.data ();
  };

  std::vector<ValueType>
  generateChildrenData (NodeType const &node)
  {
    auto results = std::vector<ValueType>{};
    std::ranges::transform (node, std::back_inserter (results), [] (auto const &node_) { return node_.data (); });
    return results;
  };
};

template <typename ValueType, typename NodeType = st_tree::detail::node_raw<st_tree::tree<ValueType>, ValueType>, typename TreeType = st_tree::tree<ValueType> > struct StTreeAdapter : public BaseTreeAdapter<StTreeNodeAdapter, ValueType, NodeType, TreeType>
{
  StTreeAdapter (st_tree::tree<ValueType> const &tree) : BaseTreeAdapter<StTreeNodeAdapter, ValueType, NodeType, TreeType>{ generateNodeAdapters (tree) } {}

  std::vector<StTreeNodeAdapter<ValueType, NodeType> >
  generateNodeAdapters (TreeType const &tree)
  {
    auto results = std::vector<StTreeNodeAdapter<ValueType, NodeType> >{};
    for (auto const &node : tree)
      {
        results.push_back (StTreeNodeAdapter<ValueType, NodeType>{ node });
      }
    return results;
  }
};

template <typename ValueType, typename ChildrenOffsetEndType = uint64_t>
inline std::expected<st_tree::tree<ValueType>, std::error_condition>
generateStTree (SmallMemoryTree<ValueType, ChildrenOffsetEndType> const &smallMemoryTree)
{
  auto const &values = smallMemoryTree.getValues ();
  auto result = st_tree::tree<ValueType>{};
  result.insert (values.front ());
  if (values.size () == 1) // only one element which means tree with only a root node
    {
      return result;
    }
  else
    {
      auto itr = result.bf_begin ();
      for (auto i = uint64_t{}; i < values.size (); ++i)
        {
          if (auto const &expectedChildrenBeginAndEndIndex = internals::childrenBeginAndEndIndex (smallMemoryTree, i))
            {
              auto const &[childBegin, childEnd] = expectedChildrenBeginAndEndIndex.value ();
              auto const &children = std::span{ values.begin () + boost::numeric_cast<int64_t> (childBegin), values.begin () + boost::numeric_cast<int64_t> (childEnd) };
              for (auto const &child : children)
                {
                  itr->insert (child);
                }
            }
          else
            {
              return std::unexpected (expectedChildrenBeginAndEndIndex.error ());
            }
          itr++;
        }
    }
  return result;
}
}