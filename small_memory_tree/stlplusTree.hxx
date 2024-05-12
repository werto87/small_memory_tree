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

namespace small_memory_tree
{

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

template <typename ValueType> struct StlplusTreeAdapter
{
  StlplusTreeAdapter (stlplus::ntree<ValueType> const &tree)
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
}