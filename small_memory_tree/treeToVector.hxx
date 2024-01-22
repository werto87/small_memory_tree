#pragma once

/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <stdexcept>

namespace small_memory_tree
{

/**
 * transforms a st_tree into a vector
 * @param tree st_tree which has a numeric data type or something which can be transformed into a number using the nodeToData function
 * @param markerForEmpty marker for empty. Will be used internally should not appear in the actual data
//TODO implement this * @param nodeToData this can be useful if your st_tree contains some data you do not want to save into the vector. Note that this is not lossless so you wont end up with exactly the same st_tree after calling vectorToTree
  @return if nodeToData is not set returns a vector containing all the information of the st_tree. If nodeToData is set the data in the vector will differ
 */
auto
treeToVector (auto const &tree, auto const &markerForEmpty)
{
  using vectorElementType = std::decay_t<decltype (markerForEmpty)>;
  auto const maxChildrenInTree = internals::getMaxChildren (tree);
  auto result = std::vector<vectorElementType>{};
  result.push_back (tree.root ().data ());
  for (auto &node : tree)
    {
      for (auto const &child : node)
        {
          result.push_back (child.data ());
        }
      for (auto addedMarkerForEmpty = uint64_t{}; (node.size () + addedMarkerForEmpty) != maxChildrenInTree; ++addedMarkerForEmpty)
        {
          result.push_back (markerForEmpty);
        }
    }
  if constexpr (internals::TupleLike<vectorElementType>)
    {
      result.push_back ({ boost::numeric_cast<std::decay_t<decltype (std::get<0> (vectorElementType{}))> > (maxChildrenInTree), {} });
    }
  else
    {
      result.push_back (boost::numeric_cast<vectorElementType> (maxChildrenInTree));
    }

  return result;
}
}
