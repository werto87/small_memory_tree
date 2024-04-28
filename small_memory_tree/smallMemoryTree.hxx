#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/createChainViews.hxx>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>

namespace small_memory_tree
{

namespace internals
{

template <typename T = uint64_t>
std::vector<T>
calculateValuesPerLevel (auto const &hierarchy, auto const &levels)
{
  if (hierarchy.empty ())
    {
      return {};
    }
  else
    {
      auto result = std::vector<T>{ hierarchy.front () };
      for (auto i = uint64_t{ 1 }; i != levels.size (); ++i)
        {
          result.push_back (boost::numeric_cast<T> (std::count (hierarchy.begin () + boost::numeric_cast<int64_t> (levels.at (i - 1)), hierarchy.begin () + boost::numeric_cast<int64_t> (levels.at (i)), true)));
        }
      std::inclusive_scan (result.begin (), result.end (), result.begin ());
      return result;
    }
}

template <typename T>
std::vector<T>
calculateLevelSmallMemoryTree (auto const &smallMemoryTreeData)
{
  auto const &[maxChildren, hierarchy, data] = smallMemoryTreeData;
  auto treeLevels = confu_algorithm::chainBreaksIncludeBreakingElement<T> (hierarchy.begin (), hierarchy.end (), [parentCount = uint64_t{ 0 }, &maxChildren] (auto cbegin, auto cend) mutable {
    if (boost::numeric_cast<uint64_t> (std::distance (cbegin, cend)) == parentCount * maxChildren || parentCount == 0)
      {
        parentCount = boost::numeric_cast<uint64_t> (std::count_if (cbegin, cend, [] (auto num) { return num != false; }));
        return false;
      }
    else
      {
        return true;
      }
  });
  return treeLevels;
}

auto
treeLevelWithOptionalValues (auto const &smallMemoryTree, uint64_t const &level, uint64_t node)
{
  auto const &levels = smallMemoryTree.getLevels ();
  if (levels.size () == level)
    {
      throw std::logic_error{ "level value is to high" };
    }
  auto const &data = smallMemoryTree.getData ();
  auto const &hierarchy = smallMemoryTree.getHierarchy ();
  if (level == 0)
    {
      if (node == 0)
        {
          return std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{ data.front () };
        }
      else
        {
          throw std::logic_error{ "node value should be 0 if level is 0. There should be only one root in the tree" };
        }
    }
  else
    {
      auto result = std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{};
      auto valuesUsed = smallMemoryTree.getValuesPerLevel ().at (level - 1);
      auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
      auto const &nodeOffsetBegin = boost::numeric_cast<int64_t> (levels.at (level - 1));
      auto const &nodeOffsetEnd = nodeOffsetBegin + maxChildren * boost::numeric_cast<int64_t> (node);
      if (nodeOffsetEnd >= boost::numeric_cast<int64_t> (levels.at (level)))
        {
          throw std::logic_error{ "node value is to high" };
        }
      valuesUsed += boost::numeric_cast<typename std::decay<decltype (valuesUsed)>::type> (std::count (hierarchy.begin () + nodeOffsetBegin, hierarchy.begin () + nodeOffsetEnd, true));
      auto processedChildren = int64_t{};
      for (auto i = nodeOffsetEnd; processedChildren != maxChildren; ++i, ++processedChildren)
        {
          if (*(hierarchy.begin () + i))
            {
              result.push_back (data.at (valuesUsed));
              valuesUsed++;
            }
          else
            {
              result.push_back ({});
            }
        }
      return result;
    }
}

}

/**
 * @brief This struct is meant to hold all the information of the tree using as less memory as possible when max children is greater 10 //TODO calculate number 10 is a guess
 *
 * @tparam DataType type of the tree elements
 * @tparam MaxChildrenType type of max children. For example if your biggest node has less than 255 children use uint8_t
 */
template <typename DataType, typename MaxChildrenType = uint64_t> struct SmallMemoryTreeData
{

  SmallMemoryTreeData (auto const &tree) : maxChildren{ boost::numeric_cast<MaxChildrenType> (internals::getMaxChildren (tree)) }, hierarchy{ internals::treeHierarchy (tree, maxChildren) }, data{ internals::treeData (tree) } {}

  MaxChildrenType maxChildren{};
  std::vector<bool> hierarchy{};
  std::vector<DataType> data{};
};

template <typename DataType, typename MaxChildrenType = uint64_t, typename LevelType = uint64_t, typename ValuesPerLevelType = uint64_t> struct SmallMemoryTree
{
public:
  SmallMemoryTree (auto smallMemoryTreeData) : _smallMemoryData{ std::move (smallMemoryTreeData) }, _levels{ internals::calculateLevelSmallMemoryTree<LevelType> (_smallMemoryData) }, _valuesPerLevel{ internals::calculateValuesPerLevel<ValuesPerLevelType> (_smallMemoryData.hierarchy, _levels) } {}

  [[nodiscard]] SmallMemoryTreeData<DataType, MaxChildrenType>
  getSmallMemoryData () const &
  {
    return _smallMemoryData;
  }

  [[nodiscard]] std::vector<LevelType> const &
  getLevels () const
  {
    return _levels;
  }

  [[nodiscard]] bool
  getMarkerForEmpty () const
  {
    return false; // resulting from the way the tree gets saved false is the marker for empty
  }

  [[nodiscard]] MaxChildrenType
  getMaxChildren () const
  {
    return _smallMemoryData.maxChildren;
  }

  [[nodiscard]] std::vector<bool> const &
  getHierarchy () const
  {
    return _smallMemoryData.hierarchy;
  }

  /**
   * creates a st_tree from the underlying vector
   * @return st_tree
   */
  st_tree::tree<DataType>
  generateTreeFromVector () const
  {
    // return internals::generateTree (_treeAsVector, _levels);
    // TODO later
    return {};
  }

  [[nodiscard]] std::vector<DataType> const &
  getData () const
  {
    return _smallMemoryData.data;
  }

  [[nodiscard]] std::vector<ValuesPerLevelType> const &
  getValuesPerLevel () const
  {
    return _valuesPerLevel;
  }

private:
  SmallMemoryTreeData<DataType, MaxChildrenType> _smallMemoryData{};
  std::vector<LevelType> _levels{};
  std::vector<ValuesPerLevelType> _valuesPerLevel{};
};

/**
 * traverses the tree by picking the first matching value
 * in this example a path with 1 and 3 would have 4 and 5 as a result
-----------0
---------/---\
--------1----1
-------/
-----3
----/-\
---4---5
 * @param treeAsVector vector in tree form
 * @param path vector with the values of nodes
 * @return value of the children of the node at the end of the path. Empty vector result means no children. Empty optional means wrong path
 */
template <typename T, typename Y, typename Z>
std::optional<std::vector<T> >
childrenByPath (SmallMemoryTree<T, Y, Z> const &smallMemoryTree, std::vector<T> const &path)
{
  auto const &levels = smallMemoryTree.getLevels ();
  if (levels.size () == 1 and path.size () == 1 and path.front () == smallMemoryTree.getData ().at (levels.at (0)))
    {
      // only one element in tree. Path is equal to the value of the element but element has no children so return empty vector
      return std::vector<T>{};
    }
  else
    {
      auto positionOfChildren = int64_t{};
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto const &valueToLookFor = path.at (i);
          auto const &levelValuesAndHoles = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTree, i, boost::numeric_cast<uint64_t> (positionOfChildren));
          auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
          if (auto itr = std::ranges::find_if (levelValuesAndHoles, [&valueToLookFor] (auto value) { return (value) && value == valueToLookFor; }); itr != levelValuesAndHoles.end ())
            {
              auto childOffset = std::distance (levelValuesAndHoles.begin (), itr);
              auto const &hierarchy = smallMemoryTree.getHierarchy ();
              positionOfChildren = std::count_if (hierarchy.begin () + ((i == 0) ? 0 : boost::numeric_cast<int64_t> (levels.at (i - 1))), hierarchy.begin () + ((i == 0) ? 0 : boost::numeric_cast<int64_t> (levels.at (i - 1))) + positionOfChildren * maxChildren + childOffset, [] (auto const &element) { return element; });
              auto const &childLevelValuesAndHoles = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTree, i + 1, boost::numeric_cast<uint64_t> (positionOfChildren));
              if (i == path.size () - 1)
                {
                  auto result = std::vector<T>{};
                  for (auto const &childValueOrHole : childLevelValuesAndHoles)
                    {
                      if (childValueOrHole)
                        {
                          result.push_back (childValueOrHole.value ());
                        }
                    }
                  return result;
                }
            }
          else
            {
              return {};
            }
        }
    }
  return {};
}
}
