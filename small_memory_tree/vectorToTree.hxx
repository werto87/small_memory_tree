#pragma once

/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <st_tree.h>

namespace small_memory_tree
{

namespace internals
{
void
addChildren (auto const &treeAsVector, auto &treeItr, auto &markerForEmpty, uint64_t maxChildren, uint64_t parentOffset)
{
  auto myChildren = children (treeAsVector, parentOffset);
  for (uint64_t i = 0; i < myChildren.size (); ++i)
    {
      auto const &child = myChildren.at (i);
      auto childIndex = uint64_t{};
      if constexpr (internals::TupleLike<std::decay_t<decltype (child)> >)
        {
          childIndex = std::get<0> (child) + i + parentOffset + 1;
        }
      else
        {
          childIndex = boost::numeric_cast<uint64_t> (child) + i + parentOffset + 1;
        }
      auto parentItr = treeItr->insert (treeAsVector[childIndex]);
      addChildren (treeAsVector, parentItr, markerForEmpty, maxChildren, childIndex);
    }
}

void
fillTree (auto const &treeAsVector, auto &tree)
{
  auto const &markerForEmpty = treeAsVector.back ();
  auto const &maxChildren = internals::maxChildren (treeAsVector);
  //  TODO this can be mostly replaced by addChildren??? kinda the same code???
  auto myChildren = children (treeAsVector, 0);
  for (uint64_t i = 0; i < myChildren.size (); ++i)
    {
      auto child = myChildren.at (i);
      auto childIndex = uint64_t{};
      if constexpr (internals::TupleLike<std::decay_t<decltype (treeAsVector.at (0))> >)
        {
          childIndex = std::get<0> (child) + i + 1;
        }
      else
        {
          childIndex = boost::numeric_cast<uint64_t> (child) + i + 1;
        }
      auto parentItr = tree.root ().insert (treeAsVector[childIndex]);
      addChildren (treeAsVector, parentItr, markerForEmpty, maxChildren, childIndex);
    }
}
}

/**
 * creates an st_tree from a vector containing a compressed st_tree
 * @param treeAsVector vector containing a compressed st_tree
 * @return decompressed st_tree
 */
auto
vectorToTree (auto const &treeAsVector)
{
  auto result = st_tree::tree<std::decay_t<decltype (treeAsVector.at (0))> >{};
  result.insert (treeAsVector.at (0));
  internals::fillTree (treeAsVector, result);
  return result;
}
}
