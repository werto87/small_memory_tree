#pragma once

/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <optional>
#include <ranges>
#include <vector>

namespace small_memory_tree
{

namespace internals
{
template <typename T>
auto
childrenByPathIterators (std::vector<T> const &treeAsVector, std::vector<T> const &path)
{
  auto maxChildren = internals::getMaxChildren (treeAsVector);
  auto results = std::vector<typename std::vector<T>::const_iterator>{};
  auto levels = internals::calculateLevels (treeAsVector);
  if (path.empty ())
    {
      for (auto i = uint64_t{}; i < maxChildren; ++i)
        {
          results.push_back (treeAsVector.begin () + 1 + i);
        }
      return results;
    }
  if (path.size () == 1)
    {
      auto children = childrenByPathIterators (treeAsVector, {});

      if (auto itr = std::find_if (children.begin (), children.end (), [valueToLookFor = path.front ()] (auto itrNode) { return *itrNode == valueToLookFor; }); itr != children.end ())
        {
          children.size ();
          std::distance (children.begin (), itr);
          results.push_back (*itr);
          return results;
        }
      else
        {
          return results;
        }
    }
  return results;
}
}

/**
 * traverses the tree by picking the first matching value
 * in this example a path with 1 and 3 would have 4 and 5 as a result
-----------0
---------/---\
--------1---2
-------/
-----3
----/-\
---4---5
 * @param treeAsVector vector in tree form
 * @param path vector with the values of nodes
 * @return value of the children of the node at the end of the path
 */
template <typename T>
std::vector<T>
childrenByPath (std::vector<T> const &treeAsVector, std::vector<T> const &path)
{
  auto result = std::vector<T>{};
  auto maxChildren = internals::getMaxChildren (treeAsVector);
  auto const &markerForEmpty = *(treeAsVector.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element
  if (path.empty ())
    {
      std::copy_if (treeAsVector.begin () + 1, treeAsVector.begin () + 1 + maxChildren, std::back_inserter (result), [&markerForEmpty] (auto const &element) { return element != markerForEmpty; });
      return result;
    }
  auto levels = internals::calculateLevels (treeAsVector); // TODO make a class and calculate this once when the vector is created
  if (path.size () == 1)
    {
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto valueToLookFor = path.at (i);
          auto level = levels.at (i);
          auto levelsWithOutEmptyElements = level | std::views::filter ([&markerForEmpty] (auto const &element) { return element != markerForEmpty; });
          if (auto itr = std::ranges::find_if (levelsWithOutEmptyElements, [&valueToLookFor] (auto value) { return value == valueToLookFor; }); itr != levelsWithOutEmptyElements.end ())
            {
              auto childrenLevel = levels.at (i + 1);
              auto position = std::distance (levelsWithOutEmptyElements.begin (), itr);
              if (i == path.size () - 1)
                {
                  for (auto j = uint64_t{}; j < maxChildren; ++j)
                    {
                      result.push_back (childrenLevel[position + j]);
                    }
                }
            }
          else
            {
              return {};
            }
        }
    }
  return result;
}
}
