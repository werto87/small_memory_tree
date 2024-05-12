#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "smallMemoryTree.hxx"
#include <st_tree.h>
namespace small_memory_tree
{

template <typename ValueType> struct StNodeAdapter
{
  StNodeAdapter () = default;
  explicit StNodeAdapter (auto node) : _data{ std::move (node.data ()) }
  {
    std::ranges::transform (node, std::back_inserter (childrenData), [] (auto const &node_) { return node_.data (); });
  }
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
    return _data;
  }

private:
  ValueType _data{};
  std::vector<ValueType> childrenData{};
};

template <typename ValueType> struct StTreeAdapter
{
  StTreeAdapter (st_tree::tree<ValueType> const &tree)
  {
    // transform does not work here because the iterator does not satisfy some concepts for example 'input_iterator' 'weakly_incrementable'
    for (auto const &node : tree)
      {
        stNodeAdapters.push_back (StNodeAdapter<ValueType>{ node });
      }
  }

  auto
  root () const
  {
    if (stNodeAdapters.empty ()) throw std::logic_error{ "empty tree has no root" };
    return stNodeAdapters.front ();
  }

  auto
  constant_breadth_first_traversal_begin () const
  {
    return stNodeAdapters.begin ();
  }

  auto
  constant_breadth_first_traversal_end () const
  {
    return stNodeAdapters.end ();
  }

private:
  std::vector<StNodeAdapter<ValueType> > stNodeAdapters{};
};

template <typename DataType, typename MaxChildrenType, typename LevelType, typename ValuesPerLevelType>
inline st_tree::tree<DataType>
generateStTree (SmallMemoryTree<DataType, MaxChildrenType, LevelType, ValuesPerLevelType> const &smallMemoryTree)
{
  auto const &data = smallMemoryTree.getData ();
  auto result = st_tree::tree<DataType>{};
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