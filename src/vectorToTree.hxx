#include "src/util.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <iterator>
#include <range/v3/algorithm.hpp>
#include <vector>
template <typename T>
void
addChildren (auto const &treeAsVector, T const &value, auto &treeItr, auto &markerForEmpty)
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
  for (auto const &child : children (treeAsVector, parentIndex, markerForEmpty))
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
      addChildren (treeAsVector, child, parentItr, markerForEmpty);
    }
}
template <typename T>
void
fillTree (auto const &treeAsVector, auto &tree, T &markerForEmpty)
{
  for (auto const &child : children (treeAsVector, 0, markerForEmpty))
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
      addChildren (treeAsVector, child, parentItr, markerForEmpty);
    }
}