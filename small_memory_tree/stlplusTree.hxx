#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "smallMemoryTree.hxx"
#include "smallMemoryTreeAdapter.hxx"
#include <cstdint>
#include <deque>
#include <stlplus/containers/ntree.hpp>
#include <type_traits>
namespace small_memory_tree
{

template <typename ValueType, typename NodeType> struct StlplusNodeAdapter : public BaseNodeAdapter<ValueType, NodeType>
{

  StlplusNodeAdapter (NodeType const &node) : BaseNodeAdapter<ValueType, NodeType>{ generateNodeData (node), generateChildrenData (node) } {}

  ValueType
  generateNodeData (NodeType const &node)
  {
    return node.m_data;
  };

  std::vector<ValueType>
  generateChildrenData (NodeType const &node)
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
  generateNodeAdapters (TreeType const &tree)
  {
    auto results = std::vector<StlplusNodeAdapter<ValueType, NodeType> >{};
    std::ranges::transform (tree.breadth_first_traversal (), std::back_inserter (results), [] (auto const &nodeWrapper) { return StlplusNodeAdapter<ValueType, NodeType>{ *nodeWrapper.node () }; });
    return results;
  }
};

// TODO do after checking how big the lookup tree is with this change
// template <typename ValueType, typename ChildrenCountType = uint64_t>
// inline stlplus::ntree<ValueType>
// generateStlplusTree (SmallMemoryTree<ValueType, ChildrenCountType> const &smallMemoryTree)
// {
//   auto const &nodes = smallMemoryTree.getNodes ();
//   auto result = stlplus::ntree<ValueType>{};
//   auto parentNodes = std::deque{ result.insert (nodes.front ().value) };
//   auto currentLevelNodes = std::decay_t<decltype (parentNodes)>{};
//   if (nodes.size () == 1) // only one element which means tree with only a root node
//     {
//       return result;
//     }
//   else
//     {
//       for (auto const &node : nodes)
//         {
//         }
//       // auto const &maxLevel = smallMemoryTree.getLevels ().size () - 1; // skipping the last level because it only has empty values by design
//       // for (auto level = uint64_t{ 1 }; level < maxLevel; ++level)      // already added the root so we start at level 1
//       //   {
//       //     auto const &currentLevel = internals::levelWithOptionalValues (smallMemoryTree, level);
//       //     for (auto node = uint64_t{}; node < currentLevel.size (); ++node)
//       //       {
//       //         if (currentLevel.at (node))
//       //           {
//       //             currentLevelNodes.push_back (result.append (parentNodes.front (), currentLevel.at (node).value ()));
//       //           }
//       //         auto const &maxChildren = smallMemoryTree.getMaxChildren ();
//       //         if (node % maxChildren == maxChildren - 1) // after processing the same amount of nodes as the amount of maxChildren we increment the itr to put the nodes under the sibling
//       //           {
//       //             parentNodes.pop_front ();
//       //           }
//       //       }
//       //     parentNodes = std::move (currentLevelNodes);
//       //     currentLevelNodes.clear (); // should be empty but it is not guaranteed by the standard
//       //   }
//     }
//   return result;
// }
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