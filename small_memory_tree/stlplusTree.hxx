#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "smallMemoryTree.hxx"
#include <stlplus/containers/ntree.hpp>
#include <stlplus/strings/print_basic.hpp>
#include <stlplus/strings/print_int.hpp>
#include <stlplus/strings/print_ntree.hpp>
#include <stlplus/strings/print_string.hpp>
#include <type_traits>

namespace small_memory_tree
{

template <typename ValueType, typename NodeType> class BaseNodeAdapter
{
public:
  BaseNodeAdapter () = delete;
  BaseNodeAdapter (ValueType const &nodeData_, std::vector<ValueType> const &childrenData_) : nodeData{ nodeData_ }, childrenData{ childrenData_ } {}

  virtual ValueType generateNodeData (NodeType const &node) = 0;

  virtual std::vector<ValueType> generateChildrenData (NodeType const &node) = 0;

  virtual ~BaseNodeAdapter () = default;

  auto
  begin () const
  {
    return childrenData.begin ();
  }
  auto
  end () const
  {
    return childrenData.end ();
  }
  size_t
  size () const
  {
    return childrenData.size ();
  }

  ValueType const &
  data () const
  {
    return nodeData;
  }

private:
  ValueType nodeData{};
  std::vector<ValueType> childrenData{};
};

template <typename ValueType, typename NodeType> struct StlplusNodeAdapter : public BaseNodeAdapter<ValueType, NodeType>
{

  StlplusNodeAdapter (NodeType const &node) : BaseNodeAdapter<ValueType, NodeType>{ generateNodeData (node), generateChildrenData (node) } {}

  ValueType
  generateNodeData (NodeType const &node) override
  {
    return node->m_data;
  };

  std::vector<ValueType>
  generateChildrenData (NodeType const &node) override
  {
    auto result = std::vector<ValueType>{};
    std::ranges::transform (node->m_children, std::back_inserter (result), [] (NodeType const &childNode) { return childNode->m_data; });
    return result;
  };
};

template <template <class, class> class NodeAdapterImpl, typename ValueType, typename NodeType, typename Tree> struct BaseTreeAdapter
{
  BaseTreeAdapter (std::vector<NodeAdapterImpl<ValueType, NodeType> > const &nodeAdapters_) : nodeAdapters{ nodeAdapters_ } {}

  std::vector<NodeAdapterImpl<ValueType, NodeType> > virtual generateNodeAdapters (Tree const &tree) = 0;
  virtual ~BaseTreeAdapter () = default;
  auto
  root () const
  {
    if (nodeAdapters.empty ()) throw std::logic_error{ "empty tree has no root" };
    return nodeAdapters.front ();
  }

  auto
  constant_breadth_first_traversal_begin () const
  {
    return nodeAdapters.begin ();
  }

  auto
  constant_breadth_first_traversal_end () const
  {
    return nodeAdapters.end ();
  }

private:
  std::vector<NodeAdapterImpl<ValueType, NodeType> > nodeAdapters{};
};

// TODO check if we really need this template parameter maybe we can write something with using
// using MyNodeType = stlplus::ntree_node<ValueType> *;
template <typename ValueType, typename NodeType = stlplus::ntree_node<ValueType> *, typename TreeType = stlplus::ntree<ValueType> > struct StlplusTreeAdapter : public BaseTreeAdapter<StlplusNodeAdapter, ValueType, NodeType, TreeType>
{
  StlplusTreeAdapter (stlplus::ntree<ValueType> const &tree) : BaseTreeAdapter<StlplusNodeAdapter, ValueType, NodeType, TreeType>{ generateNodeAdapters (tree) } {}
  // TODO implement this also for st_tree
  std::vector<StlplusNodeAdapter<ValueType, NodeType> >
  generateNodeAdapters (TreeType const &tree)
  {
    auto results = std::vector<StlplusNodeAdapter<ValueType, NodeType> >{};
    std::ranges::transform (tree.breadth_first_traversal (), std::back_inserter (results), [] (auto const &nodeWrapper) { return StlplusNodeAdapter<ValueType, NodeType>{ nodeWrapper.node () }; });
    return results;
  }
};

// TODO write generate function

}