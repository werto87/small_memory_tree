#pragma once

#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/util.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <cstddef>
#include <iterator>
#include <st_tree.h>
#include <vector>

namespace small_memory_tree
{

void
addChildren (auto const &treeAsVector, auto &treeItr, auto &markerForEmpty, uint64_t maxChildren, uint64_t parentOffset)
{
  auto myChildren = children (treeAsVector, parentOffset, markerForEmpty);
  for (uint64_t i = 0; i < myChildren.size (); ++i)
    {
      auto const &child = myChildren.at (i);
      auto childIndex = uint64_t{};
      if constexpr (TupleLike<std::decay_t<decltype (child)> >)
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
template <typename T>
void
fillTree (auto const &treeAsVector, auto &tree, T &markerForEmpty, uint64_t maxChildren)
{
  //  TODO this can be mostly replaced by addChildren??? kinda the same code???
  auto myChildren = children (treeAsVector, 0, markerForEmpty);
  for (uint64_t i = 0; i < myChildren.size (); ++i)
    {
      auto child = myChildren.at (i);
      auto childIndex = uint64_t{};
      if constexpr (TupleLike<T>)
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

template <typename T>
auto
vectorToTree (auto const &treeAsVector, T const &markerForEmpty)
{
  auto result = st_tree::tree<T>{};
  result.insert (treeAsVector.at (0));
  fillTree (treeAsVector, result, markerForEmpty, maxChildren (treeAsVector, markerForEmpty));
  return result;
}
}
