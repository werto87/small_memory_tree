#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/util.hxx"
#include <ranges>
namespace small_memory_tree
{

namespace internals
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

template <typename T>
st_tree::tree<T>
generateTree (std::vector<T> const &vectorAsTree, auto const &treeLevels)
{
  auto const &maxChildren = internals::getMaxChildren (vectorAsTree);
  auto treeToFill = st_tree::tree<T>{};
  auto trees = std::__1::deque<st_tree::tree<T> >{};
  for (auto rItr = treeLevels.crbegin (); rItr != treeLevels.crend () - 1; ++rItr)
    {
      if (rItr == treeLevels.crend () - 2)
        {
          treeToFill.insert (vectorAsTree.front ()); // the first element of treeAsVector is allways the root of the tree
          for (auto j = uint64_t{}; j < maxChildren; ++j)
            {
              treeToFill.root ().insert (trees.front ());
              trees.pop_front ();
            }
        }
      else
        {
          auto const &upperLevel = *(rItr + 1);
          auto const &markerForEmpty = *(vectorAsTree.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element
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

template <typename T>
auto
calculateLevels (std::vector<T> const &treeAsVector)
{
  uint64_t maxChildren = internals::getMaxChildren (treeAsVector);
  auto const &markerForEmpty = *(treeAsVector.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element
  auto treeLevels = confu_algorithm::createChainViewsIncludeBreakingElement (treeAsVector.begin (), treeAsVector.end () - 1, [parentCount = uint64_t{ 0 }, &maxChildren, &markerForEmpty] (auto sequence) mutable {
    if (sequence.size () == parentCount * maxChildren || parentCount == 0)
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

template <typename T> class SmallMemoryTree
{
public:
  SmallMemoryTree (auto const &st_tree, T const &markerForHoles) : _vectorAsTree{ internals::treeToVector (st_tree, markerForHoles) }, _levels{ internals::calculateLevels (_vectorAsTree) } {}

  SmallMemoryTree (auto const &st_tree, T const &markerForHoles, std::function<T (std::span<T>)>) : _vectorAsTree{ internals::treeToVector (st_tree, markerForHoles) }, _levels{ internals::calculateLevels (_vectorAsTree) } {} //    TODO implement nodeToData

  explicit SmallMemoryTree (std::vector<T> vectorAsTree) : _vectorAsTree{ std::move (vectorAsTree) }, _levels{ internals::calculateLevels (_vectorAsTree) } {}

  [[nodiscard]] std::vector<T> const &
  getVectorAsTree () const
  {
    return _vectorAsTree;
  }

  [[nodiscard]] std::vector<std::span<T const> > const &
  getLevels () const
  {
    return _levels;
  }

  [[nodiscard]] T const &
  getMarkerForEmpty () const
  {
    return *(_vectorAsTree.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element;
  }

  [[nodiscard]] uint64_t
  getMaxChildren () const
  {
    return internals::getMaxChildren (_vectorAsTree);
  }

  /**
   * creates a st_tree from the underlying vector
   * @return st_tree
   */
  st_tree::tree<T>
  generateTreeFromVector () const
  {
    return internals::generateTree (_vectorAsTree, _levels);
  }

private:
  std::vector<T> _vectorAsTree{};
  std::vector<std::span<T const> > _levels{};
};

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
childrenByPath (SmallMemoryTree<T> const &smallMemoryTree, std::vector<T> const &path)
{
  auto result = std::vector<T>{};
  auto const &levels = smallMemoryTree.getLevels ();
  auto childOffset = int64_t{};
  auto positionOfChildren = int64_t{};
  for (auto i = uint64_t{}; i < path.size (); ++i)
    {
      auto const &valueToLookFor = path.at (i);
      auto const &level = levels.at (i);
      auto nodesToChoseFrom = std::span<T const>{};
      if (i == 0)
        {
          nodesToChoseFrom = std::span<T const>{ level.begin (), level.end () };
        }
      else
        {
          nodesToChoseFrom = std::span<T const>{ level.begin () + positionOfChildren * smallMemoryTree.getMaxChildren (), level.begin () + positionOfChildren * smallMemoryTree.getMaxChildren () + smallMemoryTree.getMaxChildren () };
        }
      if (auto itr = std::ranges::find_if (nodesToChoseFrom, [&valueToLookFor] (auto value) { return value == valueToLookFor; }); itr != nodesToChoseFrom.end ())
        {
          childOffset = std::distance (nodesToChoseFrom.begin (), itr);
        }
      else
        {
          return {};
        }
      positionOfChildren = std::count_if (level.begin (), level.begin () + positionOfChildren * smallMemoryTree.getMaxChildren () + childOffset, [&markerForEmpty = smallMemoryTree.getMarkerForEmpty ()] (auto const &element) { return element != markerForEmpty; });
      auto const &childrenLevel = levels.at (i + 1);
      if (i == path.size () - 1)
        {
          for (auto j = uint64_t{}; j < smallMemoryTree.getMaxChildren (); ++j)
            {
              auto const &value = childrenLevel[positionOfChildren * smallMemoryTree.getMaxChildren () + j];
              if (value != smallMemoryTree.getMarkerForEmpty ())
                {
                  result.push_back (value);
                }
            }
        }
    }
  return result;
}

}
