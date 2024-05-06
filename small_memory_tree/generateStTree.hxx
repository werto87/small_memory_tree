#pragma once

#include "smallMemoryTree.hxx"
#include <st_tree.h>
namespace small_memory_tree
{

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