#ifndef BF56D004_36B1_4B05_B512_7D6574C01C5A
#define BF56D004_36B1_4B05_B512_7D6574C01C5A

#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/util.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <cstddef>
#include <iterator>
#include <range/v3/algorithm.hpp>
#include <st_tree.h>
#include <vector>

namespace small_memory_tree
{

template <typename T>
void
addChildren (auto const &treeAsVector, T const &value, auto &treeItr, auto &markerForEmpty, size_t maxChildren)
{
  auto parentIndex = size_t{};
  if constexpr (TupleLike<T>)
    {
      parentIndex = std::get<0> (value);
    }
  else
    {
      parentIndex = boost::numeric_cast<size_t> (value);
    }
  for (auto const &child : children (treeAsVector, parentIndex, markerForEmpty, maxChildren))
    {
      auto childIndex = size_t{};
      if constexpr (TupleLike<T>)
        {
          childIndex = std::get<0> (child);
        }
      else
        {
          childIndex = boost::numeric_cast<size_t> (child);
        }
      auto parentItr = treeItr->insert (treeAsVector[childIndex]);
      addChildren (treeAsVector, child, parentItr, markerForEmpty, maxChildren);
    }
}
template <typename T>
void
fillTree (auto const &treeAsVector, auto &tree, T &markerForEmpty, size_t maxChildren)
{
  for (auto const &child : children (treeAsVector, 0, markerForEmpty, maxChildren))
    {
      auto childIndex = size_t{};
      if constexpr (TupleLike<T>)
        {
          childIndex = std::get<0> (child);
        }
      else
        {
          childIndex = boost::numeric_cast<size_t> (child);
        }
      auto parentItr = tree.root ().insert (treeAsVector[childIndex]);
      addChildren (treeAsVector, child, parentItr, markerForEmpty, maxChildren);
    }
}

template <typename T>
auto
vectorToTree (auto const &treeAsVector, T const &markerForEmpty, size_t maxChildren)
{
  auto result = st_tree::tree<T>{};
  result.insert (treeAsVector.at (0));
  fillTree (treeAsVector, result, markerForEmpty, maxChildren);
  return result;
}
}
#endif /* BF56D004_36B1_4B05_B512_7D6574C01C5A */
