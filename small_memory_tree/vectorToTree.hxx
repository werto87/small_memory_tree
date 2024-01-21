#pragma once

/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/createChainViews.hxx>
#include <ranges>
#include <span>
#include <st_tree.h>

namespace small_memory_tree
{
template <typename T>
st_tree::tree<T>
generateTree (std::vector<T> const &treeAsVector, auto const &treeLevels)
{
  auto const &maxChildren = internals::getMaxChildren (treeAsVector);
  auto treeToFill = st_tree::tree<T>{};
  auto trees = std::__1::deque<st_tree::tree<T> >{};
  for (auto rItr = treeLevels.crbegin (); rItr != treeLevels.crend (); ++rItr)
    {
      if (rItr == treeLevels.crend () - 1)
        {
          treeToFill.insert (treeAsVector.front ()); // the first element of treeAsVector is allways the root of the tree
          for (auto j = uint64_t{}; j < maxChildren; ++j)
            {
              treeToFill.root ().insert (trees.front ());
              trees.pop_front ();
            }
        }
      else
        {
          auto const &upperLevel = *(rItr + 1);
          auto const &markerForEmpty = *(treeAsVector.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element
          auto const &notMarkerForEmpty = [&markerForEmpty] (auto const &element) { return element != markerForEmpty; };
          auto currentChild = uint64_t{};
          for (auto parent : upperLevel | std::views::filter (notMarkerForEmpty))
            {
              auto tree = st_tree::tree<T>{};
              tree.insert (parent);
              if (not trees.empty () and rItr != treeLevels.crbegin ())
                {
                  for (auto j = uint64_t{}; j < maxChildren; ++j)
                    {
                      auto const &currentLevel = *rItr;
                      if (currentLevel[currentChild] != markerForEmpty)
                        {
                          tree.root ().insert (trees.front ());
                          trees.pop_front ();
                        }
                      ++currentChild;
                    }
                }
              trees.push_back (tree);
            }
        }
    }
  return treeToFill;
}

/**
 * creates an st_tree from a vector containing a compressed st_tree
 * @param treeAsVector vector containing a compressed st_tree
 * @return decompressed st_tree
 */
template <typename T>
auto
vectorToTree (std::vector<T> const &treeAsVector)
{
  return generateTree (treeAsVector, internals::calculateLevels (treeAsVector));
}
}
