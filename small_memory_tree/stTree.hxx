#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "smallMemoryTree.hxx"
#include "smallMemoryTreeAdapter.hxx"
#include <st_tree.h>
#include <type_traits>
namespace small_memory_tree
{

template <typename ValueType, typename NodeType> struct StTreeNodeAdapter : public BaseNodeAdapter<ValueType, NodeType>
{

  StTreeNodeAdapter (NodeType const &node) : BaseNodeAdapter<ValueType, NodeType>{ generateNodeData (node), generateChildrenData (node) } {}

  ValueType
  generateNodeData (NodeType const &node) override
  {
    return node.data ();
  };

  std::vector<ValueType>
  generateChildrenData (NodeType const &node) override
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
  generateNodeAdapters (TreeType const &tree) override
  {
    auto results = std::vector<StTreeNodeAdapter<ValueType, NodeType> >{};
    for (auto const &node : tree)
      {
        results.push_back (StTreeNodeAdapter<ValueType, NodeType>{ node });
      }
    return results;
  }
};

template <typename ValueType, typename MaxChildrenType, typename LevelType, typename ValuesPerLevelType>
inline st_tree::tree<ValueType>
generateStTree (SmallMemoryTree<ValueType, MaxChildrenType, LevelType, ValuesPerLevelType> const &smallMemoryTree)
{
  auto const &data = smallMemoryTree.getData ();
  auto result = st_tree::tree<ValueType>{};
  result.insert (data.front ());
  if (data.size () == 1) // only one element which means tree with only a root node
    {
      return result;
    }
  else
    {
      auto itr = result.bf_begin ();
      auto const &maxLevel = smallMemoryTree.getLevels ().size () - 1; // skipping the last level because it only has empty values by design
      for (auto level = uint64_t{ 1 }; level < maxLevel; ++level)      // already added the root so we start at level 1
        {
          auto const &currentLevel = internals::levelWithOptionalValues (smallMemoryTree, level);
          for (auto node = uint64_t{}; node < currentLevel.size (); ++node)
            {
              if (currentLevel.at (node))
                {
                  itr->insert (currentLevel.at (node).value ());
                }
              auto const &maxChildren = smallMemoryTree.getMaxChildren ();
              if (node % maxChildren == maxChildren - 1) // after processing the same amount of nodes as the amount of maxChildren we increment the itr to put the nodes under the sibling
                {
                  itr++;
                }
            }
        }
    }
  return result;
}
}