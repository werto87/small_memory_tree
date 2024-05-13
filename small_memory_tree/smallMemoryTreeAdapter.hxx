#pragma once

#include <stdexcept>

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
}