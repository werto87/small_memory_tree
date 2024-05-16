#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/createChainViews.hxx>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
namespace small_memory_tree
{

namespace internals
{

template <typename T> concept IsNode = requires (T const a)
{
  {
    a.begin ()
  } -> std::forward_iterator;
  {
    a.end ()
  } -> std::forward_iterator;
  { a.size () };
  { a.data () };
};

// TODO This does not check what kind of iterator it is.
// if we find out how to check if it is a breath first iterator or not we can rename this
template <typename T> concept HasIteratorToNode = requires (T a)
{
  { a.root () };
  { ++a.constant_breadth_first_traversal_begin () };
  { ++a.constant_breadth_first_traversal_end () };
  {
    *a.constant_breadth_first_traversal_begin ()
  } -> IsNode;
  {
    *a.constant_breadth_first_traversal_end ()
  } -> IsNode;
};

[[nodiscard]] uint64_t
calculateMaxChildren (HasIteratorToNode auto const &tree)
{
  auto maxChildren = uint64_t{};
  std::for_each (tree.constant_breadth_first_traversal_begin (), tree.constant_breadth_first_traversal_end (), [&maxChildren] (auto const &node) {
    if (maxChildren < node.size ())
      {
        maxChildren = node.size ();
      }
  });
  return maxChildren;
}

[[nodiscard]] auto
treeData (HasIteratorToNode auto const &tree)
{
  typedef typename std::decay<decltype (tree.root ().data ())>::type TreeDataElementType;
  auto results = std::vector<TreeDataElementType>{};
  std::for_each (tree.constant_breadth_first_traversal_begin (), tree.constant_breadth_first_traversal_end (), [&results] (auto const &node) { results.push_back (node.data ()); });
  return results;
}

[[nodiscard]] inline std::vector<bool>
treeHierarchy (HasIteratorToNode auto const &tree, uint64_t maxChildrenInTree)
{
  auto result = std::vector<bool>{};
  result.push_back (true);
  std::for_each (tree.constant_breadth_first_traversal_begin (), tree.constant_breadth_first_traversal_end (), [&result, maxChildrenInTree] (auto const &node) {
    std::ranges::for_each (node, [&result] (auto const &) { result.push_back (true); });
    for (auto addedMarkerForEmpty = uint64_t{}; (node.size () + addedMarkerForEmpty) != maxChildrenInTree; ++addedMarkerForEmpty)
      {
        result.push_back (false);
      }
  });
  return result;
}

template <typename T = uint64_t>
[[nodiscard]] std::vector<T>
calculateValuesPerLevel (auto const &hierarchy, auto const &levels)
{
  if (hierarchy.empty ())
    {
      return {};
    }
  else
    {
      auto result = std::vector<T>{ hierarchy.front () };
      for (auto i = uint64_t{ 1 }; i != levels.size (); ++i)
        {
          result.push_back (boost::numeric_cast<T> (std::count (hierarchy.begin () + boost::numeric_cast<int64_t> (levels.at (i - 1)), hierarchy.begin () + boost::numeric_cast<int64_t> (levels.at (i)), true)));
        }
      std::inclusive_scan (result.begin (), result.end (), result.begin ());
      return result;
    }
}

template <typename T>
std::vector<T>
calculateLevelSmallMemoryTree (auto const &smallMemoryTreeData)
{
  auto const &[maxChildren, hierarchy, data] = smallMemoryTreeData;
  auto treeLevels = confu_algorithm::chainBreaksIncludeBreakingElement<T> (hierarchy.begin (), hierarchy.end (), [parentCount = uint64_t{ 0 }, &maxChildren] (auto cbegin, auto cend) mutable {
    if (boost::numeric_cast<uint64_t> (std::distance (cbegin, cend)) == parentCount * maxChildren || parentCount == 0)
      {
        parentCount = boost::numeric_cast<uint64_t> (std::count_if (cbegin, cend, [] (auto num) { return num != false; }));
        return false;
      }
    else
      {
        return true;
      }
  });
  return treeLevels;
}

[[nodiscard]] auto
levelWithOptionalValues (auto const &smallMemoryTree, uint64_t const &level)
{
  auto const &levels = smallMemoryTree.getLevels ();
  if (levels.size () == level)
    {
      throw std::logic_error{ "level value is to high" };
    }
  auto const &data = smallMemoryTree.getData ();
  auto const &hierarchy = smallMemoryTree.getHierarchy ();
  if (level == 0)
    {
      return std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{ data.front () };
    }
  else
    {
      auto result = std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{};
      auto const &nodeOffsetBegin = boost::numeric_cast<typename std::decay<decltype (levels.front ())>::type> (levels.at (level - 1));
      auto valuesUsed = smallMemoryTree.getTotalValuesUsedUntilLevel ().at (level - 1);
      for (auto i = nodeOffsetBegin; i != levels.at (level); ++i)
        {
          if (*(hierarchy.begin () + boost::numeric_cast<int64_t> (i)))
            {
              result.push_back (data.at (valuesUsed));
              valuesUsed++;
            }
          else
            {
              result.push_back ({});
            }
        }
      return result;
    }
}

[[nodiscard]] auto
childrenWithOptionalValues (auto const &smallMemoryTree, uint64_t const &level, uint64_t node)
{
  auto const &levels = smallMemoryTree.getLevels ();
  if (levels.size () == level)
    {
      throw std::logic_error{ "level value is to high" };
    }
  auto const &data = smallMemoryTree.getData ();
  auto const &hierarchy = smallMemoryTree.getHierarchy ();
  if (level == 0)
    {
      if (node == 0)
        {
          return std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{ data.front () };
        }
      else
        {
          throw std::logic_error{ "node value should be 0 if level is 0. There should be only one root in the tree" };
        }
    }
  else
    {
      auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
      auto const &nodeOffsetBegin = boost::numeric_cast<int64_t> (levels.at (level - 1));
      auto const &nodeOffsetEnd = nodeOffsetBegin + maxChildren * boost::numeric_cast<int64_t> (node);
      if (nodeOffsetEnd >= boost::numeric_cast<int64_t> (levels.at (level)))
        {
          throw std::logic_error{ "node value is to high" };
        }
      auto processedChildren = int64_t{};
      auto valuesUsed = smallMemoryTree.getTotalValuesUsedUntilLevel ().at (level - 1) + boost::numeric_cast<typename std::decay<decltype (smallMemoryTree.getTotalValuesUsedUntilLevel ().at (level - 1))>::type> (std::count (hierarchy.begin () + nodeOffsetBegin, hierarchy.begin () + nodeOffsetEnd, true));
      auto result = std::vector<std::optional<typename std::decay<decltype (data.front ())>::type> >{};
      result.reserve (boost::numeric_cast<uint64_t> (maxChildren));
      for (auto i = nodeOffsetEnd; processedChildren != maxChildren; ++i, ++processedChildren)
        {
          if (*(hierarchy.begin () + i))
            {
              result.push_back (data.at (valuesUsed));
              valuesUsed++;
            }
          else
            {
              result.push_back (std::nullopt);
            }
        }
      return result;
    }
}

}

/**
 * @brief This struct is meant to hold all the information of the tree using as less memory as possible
 *
 * @tparam DataType type of the tree elements
 * @tparam MaxChildrenType type of max children. For example if your biggest node has less than 255 children use uint8_t
 */

template <typename DataType, typename MaxChildrenType = uint64_t> struct SmallMemoryTreeData
{

  template <internals::HasIteratorToNode Tree> SmallMemoryTreeData (Tree const &tree) : maxChildren{ boost::numeric_cast<MaxChildrenType> (internals::calculateMaxChildren (tree)) }, hierarchy{ internals::treeHierarchy (tree, maxChildren) }, data{ internals::treeData (tree) } {}

  // clang-format off
    [[nodiscard]]
  auto operator<=> (const SmallMemoryTreeData &) const = default;
  // clang-format on

  MaxChildrenType maxChildren{};
  std::vector<bool> hierarchy{};
  std::vector<DataType> data{};
};

/**
 * @brief Extends small memory tree data by adding caching variables which are used for value lookup when using childrenByPath function and generate st tree function
 *
 * @tparam DataType type of the tree elements
 * @tparam MaxChildrenType type of max children. For example if your biggest node has less than 255 children use uint8_t
 * @tparam LevelType lookup to find out which node has which value. saves indicies to denote where a level begins and ends. If you are sure that you know the max tree width you can change the type to a better fitting type and maybe save some memory. I do not think it will lead to savings in storage because databases are good in optimizing this and use only the needed amount of storage
 * @tparam ValuesPerLevelType lookup to find out which node has which value. saves values used until this level. If you are sure that you know the max tree width you can change the type to a better fitting type and maybe save some memory. I do not think it will lead to savings in storage because databases are good in optimizing this and use only the needed amount of storage
 */
template <typename DataType, typename MaxChildrenType = uint64_t, typename LevelType = uint64_t, typename ValuesPerLevelType = uint64_t> struct SmallMemoryTree
{
public:
  explicit SmallMemoryTree (SmallMemoryTreeData<DataType, MaxChildrenType> smallMemoryTreeData) : _smallMemoryData{ std::move (smallMemoryTreeData) }, _levels{ internals::calculateLevelSmallMemoryTree<LevelType> (_smallMemoryData) }, _valuesPerLevel{ internals::calculateValuesPerLevel<ValuesPerLevelType> (_smallMemoryData.hierarchy, _levels) } {}

  // clang-format off
  [[nodiscard]]
  auto operator<=> (const SmallMemoryTree &)  const noexcept = default;
  // clang-format on

  // TODO there should be an option to sort the children so we can use binary search
  [[nodiscard]] SmallMemoryTreeData<DataType, MaxChildrenType>
  getSmallMemoryData () const &noexcept
  {
    return _smallMemoryData;
  }

  [[nodiscard]] std::vector<LevelType> const &
  getLevels () const noexcept
  {
    return _levels;
  }

  [[nodiscard]] bool
  getMarkerForEmpty () const noexcept
  {
    return false; // resulting from the way the tree gets saved false is the marker for empty
  }

  [[nodiscard]] MaxChildrenType
  getMaxChildren () const noexcept
  {
    return _smallMemoryData.maxChildren;
  }

  [[nodiscard]] std::vector<bool> const &
  getHierarchy () const noexcept
  {
    return _smallMemoryData.hierarchy;
  }

  [[nodiscard]] std::vector<DataType> const &
  getData () const noexcept
  {
    return _smallMemoryData.data;
  }

  [[nodiscard]] std::vector<ValuesPerLevelType> const &
  getTotalValuesUsedUntilLevel () const noexcept
  {
    return _valuesPerLevel;
  }

private:
  SmallMemoryTreeData<DataType, MaxChildrenType> _smallMemoryData{};
  std::vector<LevelType> _levels{};
  std::vector<ValuesPerLevelType> _valuesPerLevel{};
};

/**
 * traverses the tree by picking the first matching value
 * in this example a path with 0 1 and 3 would have 4 and 5 as a result
-----------0
---------/---\
--------1----1
-------/
-----3
----/-\
---4---5
 * @param smallMemoryTree vector in tree form
 * @param path vector with the values of nodes
 * @return value of the children of the node at the end of the path. Empty vector result means no children. Empty optional means wrong path
 */
template <typename DataType, typename MaxChildrenType, typename LevelType, typename ValuesPerLevelType>
std::optional<std::vector<DataType> >
childrenByPath (SmallMemoryTree<DataType, MaxChildrenType, LevelType, ValuesPerLevelType> const &smallMemoryTree, std::vector<DataType> const &path)
{
  // TODO this has bad performance
  auto const &levels = smallMemoryTree.getLevels ();
  if (levels.size () == 1 and path.size () == 1 and path.front () == smallMemoryTree.getData ().at (levels.at (0)))
    {
      // only one element in tree. Path is equal to the value of the element but element has no children so return empty vector
      return std::vector<DataType>{};
    }
  else
    {
      auto positionOfChildren = int64_t{};
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto const &valueToLookFor = path.at (i);
          auto const &childrenValuesAndHoles = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, i, boost::numeric_cast<uint64_t> (positionOfChildren));
          auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
          if (auto itr = std::find_if (childrenValuesAndHoles.begin (), childrenValuesAndHoles.end (), [valueToLookFor] (auto const &value) { return (value) && value == valueToLookFor; }); itr != childrenValuesAndHoles.end ())
            {
              auto const childOffset = std::distance (childrenValuesAndHoles.begin (), itr);
              auto const &hierarchy = smallMemoryTree.getHierarchy ();
              positionOfChildren = std::count_if (hierarchy.begin () + ((i == 0) ? 0 : boost::numeric_cast<int64_t> (levels.at (i - 1))), hierarchy.begin () + ((i == 0) ? 0 : boost::numeric_cast<int64_t> (levels.at (i - 1))) + positionOfChildren * maxChildren + childOffset, [] (auto const &element) { return element; });
              if (i == path.size () - 1)
                {
                  auto const &childLevelValuesAndHoles = small_memory_tree::internals::childrenWithOptionalValues (smallMemoryTree, i + 1, boost::numeric_cast<uint64_t> (positionOfChildren));
                  auto result = std::vector<DataType>{};
                  for (auto const &childValueOrHole : childLevelValuesAndHoles)
                    {
                      if (childValueOrHole)
                        {
                          result.push_back (childValueOrHole.value ());
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
}
