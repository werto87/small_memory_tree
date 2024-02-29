#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/smallMemoryTree.hxx"
#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <concepts>
#include <confu_algorithm/createChainViews.hxx>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
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
          result.push_back (boost::numeric_cast<T> (std::count (hierarchy.begin () + levels.at (i - 1), hierarchy.begin () + levels.at (i), true)));
        }
      std::inclusive_scan (result.begin (), result.end (), result.begin ());
      return result;
    }
}

template <typename T>
std::vector<T>
calculateLevelSmallMemoryTreeLotsOfChildrenData (auto const &smallMemoryTreeLotsOfChildrenData)
{
  auto const &[hierarchy, data, maxChildren] = smallMemoryTreeLotsOfChildrenData;
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
treeLevelWithOptionalValues (auto const &smallMemoryTreeLotsOfChildren, uint64_t const &level)
{
  auto const &levels = smallMemoryTreeLotsOfChildren.getLevels ();
  auto const &data = smallMemoryTreeLotsOfChildren.getData ();
  auto const &hierarchy = smallMemoryTreeLotsOfChildren.getHierarchy ();
  if (level == 0)
    {
      return std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{ data.front () };
    }
  else
    {
      auto result = std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{};
      auto valuesUsed = smallMemoryTreeLotsOfChildren.getValuesPerLevel ().at (level - 1);
      for (auto i = int64_t{ levels.at (level - 1) }; i != levels.at (level); ++i)
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
 * @tparam T type of the tree elements
 * @tparam Y type of max children. For example if your biggest node has less than 255 children use uint8_t
 */
template <typename T, typename Y> struct SmallMemoryTreeLotsOfChildrenData
{

  SmallMemoryTreeLotsOfChildrenData (auto const &tree) : hierarchy{ internals::treeHierarchy (tree) }, data{ internals::treeData (tree) }, maxChildren{ boost::numeric_cast<Y> (internals::getMaxChildren (tree)) } {}

  std::vector<bool> hierarchy{};
  std::vector<T> data{};
  Y maxChildren{};
};

template <typename T, typename Y, typename LevelType = uint64_t, typename ValuesPerLevelType = uint64_t> struct SmallMemoryTreeLotsOfChildren
{
public:
  SmallMemoryTreeLotsOfChildren (auto smallMemoryTreeLotsOfChildrenData) : _smallMemoryTreeLotsOfChildrenData{ std::move (smallMemoryTreeLotsOfChildrenData) }, _levels{ internals::calculateLevelSmallMemoryTreeLotsOfChildrenData<LevelType> (_smallMemoryTreeLotsOfChildrenData) }, _valuesPerLevel{ internals::calculateValuesPerLevel<ValuesPerLevelType> (_smallMemoryTreeLotsOfChildrenData.hierarchy, _levels) } {}

  [[nodiscard]] SmallMemoryTreeLotsOfChildrenData<T, Y>
  createSmallMemoryTreeLotsOfChildrenData () const &
  {
    return _smallMemoryTreeLotsOfChildrenData;
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

  [[nodiscard]] Y
  getMaxChildren () const
  {
    return _smallMemoryTreeLotsOfChildrenData.maxChildren;
  }

  [[nodiscard]] std::vector<bool>
  getHierarchy () const
  {
    return _smallMemoryTreeLotsOfChildrenData.hierarchy;
  }

  /**
   * creates a st_tree from the underlying vector
   * @return st_tree
   */
  st_tree::tree<T>
  generateTreeFromVector () const
  {
    // return internals::generateTree (_treeAsVector, _levels);
    // TODO later
    return {};
  }

  [[nodiscard]] std::vector<T> const &
  getData () const
  {
    return _smallMemoryTreeLotsOfChildrenData.data;
  }

  [[nodiscard]] std::vector<ValuesPerLevelType> const &
  getValuesPerLevel () const
  {
    return _valuesPerLevel;
  }

private:
  SmallMemoryTreeLotsOfChildrenData<T, Y> _smallMemoryTreeLotsOfChildrenData{};
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
childrenByPath (SmallMemoryTreeLotsOfChildren<T, Y, Z> const &smallMemoryTreeLotsOfChildren, std::vector<T> const &path)
{
  auto const &levels = smallMemoryTreeLotsOfChildren.getLevels ();
  if (levels.size () == 1 and path.size () == 1 and path.front () == smallMemoryTreeLotsOfChildren.getData ().at (levels.at (0)))
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
          // The next function could create millions of elements we do not need
          auto const &levelValuesAndHoles = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, i);
          auto valuesAndHoles = std::span{ levelValuesAndHoles.cbegin (), levelValuesAndHoles.cend () };
          auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTreeLotsOfChildren.getMaxChildren ());
          if (i != 0)
            {
              valuesAndHoles = std::span{ levelValuesAndHoles.begin () + positionOfChildren * maxChildren, levelValuesAndHoles.begin () + positionOfChildren * maxChildren + maxChildren };
            }
          if (auto itr = std::ranges::find_if (valuesAndHoles, [&valueToLookFor] (auto value) { return (value) && value == valueToLookFor; }); itr != valuesAndHoles.end ())
            {
              auto childOffset = std::distance (valuesAndHoles.begin (), itr);
              positionOfChildren = std::count_if (levelValuesAndHoles.begin (), levelValuesAndHoles.begin () + positionOfChildren * maxChildren + childOffset, [] (auto const &element) { return element; });
              auto const &childLevelValuesAndHoles = small_memory_tree::internals::treeLevelWithOptionalValues (smallMemoryTreeLotsOfChildren, i + 1);
              if (i == path.size () - 1)
                {
                  auto result = std::vector<T>{};
                  for (auto j = int64_t{}; j < maxChildren; ++j)
                    {
                      if (childLevelValuesAndHoles[(boost::numeric_cast<uint64_t> (positionOfChildren * maxChildren + j))])
                        {
                          result.push_back (childLevelValuesAndHoles[(boost::numeric_cast<uint64_t> (positionOfChildren * maxChildren + j))].value ());
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
