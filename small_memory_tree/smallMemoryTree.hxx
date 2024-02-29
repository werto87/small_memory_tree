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
 * @param nodeToData this can be useful if your st_tree contains some data you do not want to save into the vector. Note that this is not lossless so you wont end up with exactly the same st_tree after calling vectorToTree
   @return if nodeToData is not set returns a vector containing all the information of the st_tree. If nodeToData is set the data in the vector will differ
 */
template <typename T>
auto
treeToVector (auto const &tree, T const &markerForEmpty, std::function<typename std::decay<decltype (markerForEmpty)>::type (typename std::decay<decltype (*tree.begin ())>::type const &node)> nodeToData = {})
{
  auto const maxChildrenInTree = internals::getMaxChildren (tree);
  auto result = std::vector<T>{};
  if (nodeToData)
    {
      result.push_back (nodeToData (tree.root ()));
    }
  else
    {
      if constexpr (std::is_same<typename std::decay<decltype (tree.root ().data ())>::type, T>::value)
        {
          result.push_back (tree.root ().data ());
        }
      else
        {
          throw std::logic_error{ "node type and element type of result vector is not the same." };
        }
    }
  for (auto &node : tree)
    {
      for (auto const &child : node)
        {
          if (nodeToData)
            {
              result.push_back (nodeToData (child));
            }
          else
            {
              if constexpr (std::is_same<typename std::decay<decltype (child.data ())>::type, T>::value)
                {
                  result.push_back (child.data ());
                }
              else
                {
                  throw std::logic_error{ "node type and element type of result vector is not the same." };
                }
            }
        }
      for (auto addedMarkerForEmpty = uint64_t{}; (node.size () + addedMarkerForEmpty) != maxChildrenInTree; ++addedMarkerForEmpty)
        {
          result.push_back (markerForEmpty);
        }
    }
  if constexpr (internals::TupleLike<T>)
    {
      result.push_back ({ boost::numeric_cast<std::decay_t<decltype (std::get<0> (T{}))> > (maxChildrenInTree), {} });
    }
  else
    {
      result.push_back (boost::numeric_cast<T> (maxChildrenInTree));
    }
  return result;
}

template <typename T>
st_tree::tree<T>
generateTree (std::vector<T> const &treeAsVector, auto const &treeLevels)
{
  auto const &maxChildren = internals::getMaxChildren (treeAsVector);
  auto treeToFill = st_tree::tree<T>{};
  auto trees = std::deque<st_tree::tree<T> >{};
  for (auto rItr = treeLevels.crbegin (); rItr != treeLevels.crend () - 1; ++rItr)
    {
      if (rItr == treeLevels.crend () - 2)
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

template <typename T>
auto
calculateLevels (std::vector<T> &treeAsVector)
{
  uint64_t maxChildren = internals::getMaxChildren (treeAsVector);
  auto const &markerForEmpty = *(treeAsVector.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element
  auto treeLevels = confu_algorithm::createChainViewsIncludeBreakingElement (treeAsVector.cbegin (), treeAsVector.cend () - 1, [parentCount = uint64_t{ 0 }, &maxChildren, &markerForEmpty] (auto cbegin, auto cend) mutable {
    if (boost::numeric_cast<uint64_t> (std::distance (cbegin, cend)) == parentCount * maxChildren || parentCount == 0)
      {
        parentCount = boost::numeric_cast<uint64_t> (std::count_if (cbegin, cend, [&markerForEmpty] (auto num) { return num != markerForEmpty; }));
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
  SmallMemoryTree (auto const &st_tree, T const &markerForEmpty) : _treeAsVector{ internals::treeToVector (st_tree, markerForEmpty) }, _levels{ internals::calculateLevels (_treeAsVector) } {}

  SmallMemoryTree (auto const &st_tree, T const &markerForEmpty, std::function<typename std::decay<decltype (markerForEmpty)>::type (typename std::decay<decltype (*st_tree.begin ())>::type const &node)> nodeToData) : _treeAsVector{ internals::treeToVector (st_tree, markerForEmpty, nodeToData) }, _levels{ internals::calculateLevels (_treeAsVector) } {}

  explicit SmallMemoryTree (std::vector<T> treeAsVector) : _treeAsVector{ std::move (treeAsVector) }, _levels{ internals::calculateLevels (_treeAsVector) } {}

  [[nodiscard]] std::vector<T> const &
  getTreeAsVector () const
  {
    return _treeAsVector;
  }

  [[nodiscard]] std::vector<std::span<T const> > const &
  getLevels () const
  {
    return _levels;
  }

  [[nodiscard]] T const &
  getMarkerForEmpty () const
  {
    return *(_treeAsVector.end () - 2); // resulting from the way the tree gets saved in the vector the marker for empty will be allways the second last element;
  }

  [[nodiscard]] uint64_t
  getMaxChildren () const
  {
    return internals::getMaxChildren (_treeAsVector);
  }

  /**
   * creates a st_tree from the underlying vector
   * @return st_tree
   */
  st_tree::tree<T>
  generateTreeFromVector () const
  {
    return internals::generateTree (_treeAsVector, _levels);
  }

private:
  std::vector<T> _treeAsVector{};
  std::vector<std::span<T const> > _levels{};
};

/**
 * traverses the tree by picking the first matching value
 * in this example a path with 1 and 3 would have 4 and 5 as a result
-----------0
---------/---\
--------1----1
-------/
-----3
----/-\
---4---5
 * @param treeAsVector vector in tree form
 * @param path vector with the values of nodes
 * @return value of the children of the node at the end of the path. Empty vector result means no children. Empty optional means wrong path
 */
template <typename T>
std::optional<std::vector<T> >
childrenByPath (SmallMemoryTree<T> const &smallMemoryTree, std::vector<T> const &path)
{
  auto const &levels = smallMemoryTree.getLevels ();
  if (levels.size () == 1 and path.size () == 1 and path.front () == levels.at (0).front ())
    {
      // only one element in tree. Path is equal to the value of the element but element has no children so return empty vector
      return std::vector<T>{};
    }
  else
    {
      auto positionOfChildren = int64_t{};
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto const &valueToLookFor = path.at (i);
          auto const &level = levels.at (i);
          auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
          auto nodesToChoseFrom = level;
          if (i != 0)
            {
              nodesToChoseFrom = std::span<T const>{ level.begin () + positionOfChildren * maxChildren, level.begin () + positionOfChildren * maxChildren + maxChildren };
            }
          if (auto itr = std::ranges::find_if (nodesToChoseFrom, [&valueToLookFor] (auto value) { return value == valueToLookFor; }); itr != nodesToChoseFrom.end ())
            {
              auto childOffset = std::distance (nodesToChoseFrom.begin (), itr);
              positionOfChildren = std::count_if (level.begin (), level.begin () + positionOfChildren * maxChildren + childOffset, [&markerForEmpty = smallMemoryTree.getMarkerForEmpty ()] (auto const &element) { return element != markerForEmpty; });
              auto const &childrenLevel = levels.at (i + 1);
              if (i == path.size () - 1)
                {
                  auto result = std::vector<T>{};
                  for (auto j = int64_t{}; j < maxChildren; ++j)
                    {
                      auto const &value = childrenLevel[boost::numeric_cast<uint64_t> (positionOfChildren * maxChildren + j)];
                      if (value != smallMemoryTree.getMarkerForEmpty ())
                        {
                          result.push_back (value);
                        }
                    }
                  return result;
                }
            }
          else
            {
              return {};
            }
        }
    }
  return {};
}

/**
 * @param smallMemoryTree
 * @return Root element of the tree. Empty optional if tree is empty
 */
template <typename T>
std::optional<T>
rootElement (SmallMemoryTree<T> const &smallMemoryTree)
{
  if (smallMemoryTree.getTreeAsVector ().empty ())
    {
      return {};
    }
  else
    {
      return { smallMemoryTree.getTreeAsVector ().front () };
    }
}

}
