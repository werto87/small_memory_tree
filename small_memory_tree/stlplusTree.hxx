#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

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
  // TODO implement this also for st_tree
  std::vector<StlplusNodeAdapter<ValueType, NodeType> >
  generateNodeAdapters (TreeType const &tree) override
  {
    auto results = std::vector<StlplusNodeAdapter<ValueType, NodeType> >{};
    std::ranges::transform (tree.breadth_first_traversal (), std::back_inserter (results), [] (auto const &nodeWrapper) { return StlplusNodeAdapter<ValueType, NodeType>{ *nodeWrapper.node () }; });
    return results;
  }
};

// TODO write generate function

}