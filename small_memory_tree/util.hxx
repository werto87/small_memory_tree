#pragma once

/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/createChainViews.hxx>
#include <tuple>

namespace small_memory_tree::internals
{

// indent_padding and serialize_indented are copied from https://github.com/erikerlandson/st_tree/blob/develop/examples/x_common.h
inline const char *
indent_padding (unsigned n)
{
  static char const spaces[] = "                                                                   ";
  static const unsigned ns = sizeof (spaces) / sizeof (*spaces);
  if (n >= ns) n = ns - 1;
  return spaces + (ns - 1 - n);
}

template <typename Iterator, typename Stream>
void
serialize_indented (const Iterator &F, const Iterator &L, Stream &s, unsigned indent = 2)
{
  for (Iterator j (F); j != L; ++j)
    {
      s << indent_padding (j->ply () * indent);
      s << j->data ();
      s << '\n';
    }
}

// use this to get type at compile time useful to check types in templated code
// WhichType<Something>{};
template <typename...> struct WhichType;

template <typename T> concept TupleLike = requires (T a)
{
  std::tuple_size<T>::value;
  std::get<0> (a);
};

template <typename T>
uint64_t
getMaxChildren (T const &tree)
{
  auto maxChildren = uint64_t{};
  for (auto const &node : tree)
    {
      if (maxChildren < node.size ())
        {
          maxChildren = node.size ();
        }
    }
  return maxChildren;
}

template <typename T>
uint64_t
getMaxChildren (const std::vector<T> &treeAsVector)
{
  auto maxChildren = uint64_t{}; // last element is the maxChildren value of the tree
  if constexpr (TupleLike<T>)
    {
      maxChildren = boost::numeric_cast<uint64_t> (std::get<0> (treeAsVector.back ()));
    }
  else
    {
      maxChildren = boost::numeric_cast<uint64_t> (treeAsVector.back ());
    }
  return maxChildren;
}

// TODO add a parameter so it is possible to calculate only a couple levels and not always all levels
// TODO this currently does ignore the first level which is allways the root do not know if it is good or bad
template <typename T>
auto
calculateLevels (std::vector<T> const &treeAsVector)
{
  uint64_t maxChildren = internals::getMaxChildren (treeAsVector);
  auto const &markerForEmpty = *(treeAsVector.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element
  auto treeLevels = confu_algorithm::createChainViewsIncludeBreakingElement (treeAsVector.begin () + 1, treeAsVector.end () - 1, [parentCount = uint64_t{ 1 }, &maxChildren, &markerForEmpty] (auto sequence) mutable {
    if (sequence.size () == parentCount * maxChildren)
      {
        parentCount = boost::numeric_cast<uint64_t> (std::count_if (sequence.begin (), sequence.end (), [&markerForEmpty] (auto num) { return num != markerForEmpty; }));
        return false;
      }
    else
      {
        return true;
      }
  });
  return treeLevels;
}

}
