#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "small_memory_tree/treeToVector.hxx"
#include "small_memory_tree/util.hxx"
#include <ranges>
namespace small_memory_tree
{

namespace internals
{
template <typename T>
st_tree::tree<T>
generateTree (std::vector<T> const &vectorAsTree, auto const &treeLevels)
{
  auto const &maxChildren = internals::getMaxChildren (vectorAsTree);
  auto treeToFill = st_tree::tree<T>{};
  auto trees = std::__1::deque<st_tree::tree<T> >{};
  for (auto rItr = treeLevels.crbegin (); rItr != treeLevels.crend (); ++rItr)
    {
      if (rItr == treeLevels.crend () - 1)
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
template <typename T> class SmallMemoryTree
{
public:
  SmallMemoryTree (auto const &st_tree, T const &markerForHoles) : _vectorAsTree{ treeToVector (st_tree, markerForHoles) }, _levels{ internals::calculateLevels (_vectorAsTree) } {}

  SmallMemoryTree (auto const &st_tree, T const &markerForHoles, std::function<T (std::span<T>)>) : _vectorAsTree{ treeToVector (st_tree, markerForHoles) }, _levels{ internals::calculateLevels (_vectorAsTree) } {} //    TODO implement nodeToData

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
   * creates an st_tree from the underlying vector
   * @return decompressed st_tree
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
  if (path.size () == 1)
    {
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto valueToLookFor = path.at (i);
          auto level = levels.at (i);
          auto levelsWithOutEmptyElements = level | std::views::filter ([&markerForEmpty = smallMemoryTree.getMarkerForEmpty ()] (auto const &element) { return element != markerForEmpty; });
          if (auto itr = std::ranges::find_if (levelsWithOutEmptyElements, [&valueToLookFor] (auto value) { return value == valueToLookFor; }); itr != levelsWithOutEmptyElements.end ())
            {
              auto childrenLevel = levels.at (i + 1);
              auto position = std::distance (levelsWithOutEmptyElements.begin (), itr);
              if (i == path.size () - 1)
                {
                  for (auto j = uint64_t{}; j < smallMemoryTree.getMaxChildren (); ++j)
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
