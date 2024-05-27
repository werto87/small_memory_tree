#pragma once
/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <confu_algorithm/binaryFind.hxx>
#include <confu_algorithm/createChainViews.hxx>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
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

template <typename ValueType = uint64_t>
[[nodiscard]] std::vector<std::vector<ValueType> >
calculateNodeIndexesPerLevel (auto const &hierarchy, auto const &levels)
{
  if (hierarchy.empty ())
    {
      return {};
    }
  else
    {
      auto result = std::vector<std::vector<ValueType> > (levels.size ());
      result.at (0).push_back (0); // Root element is always on position 1
      for (auto i = uint64_t{ 1 }; i != levels.size (); ++i)
        {
          auto levelBegin = hierarchy.begin () + boost::numeric_cast<int64_t> (levels.at (i - 1));
          auto levelEnd = hierarchy.begin () + boost::numeric_cast<int64_t> (levels.at (i));
          for (auto hierarchyItr = std::find (levelBegin, levelEnd, true); hierarchyItr != levelEnd;)
            {
              result.at (i).push_back (boost::numeric_cast<ValueType> (std::distance (levelBegin, hierarchyItr)));
              ++hierarchyItr;
              hierarchyItr = std::find (hierarchyItr, levelEnd, true);
            }
        }
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
levelWithOptionalValues (auto const &smallMemoryTree, uint64_t level)
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
      auto valuesUsed = smallMemoryTree.getTotalValuesUsedUntilLevel (level);
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
// TODO try to move code into some functions this is to complicated
[[nodiscard]] auto
childrenAndUsedValuesUntilChildren (auto const &smallMemoryTree, uint64_t level, uint64_t node)
{
  auto const &levels = smallMemoryTree.getLevels ();
  auto const &data = smallMemoryTree.getData ();
  using ResultType = std::tuple<std::vector<std::decay_t<decltype (data.front ())> >, int64_t>;
  if (levels.size () == 1 and level == 0 and node == 0)
    {
      return ResultType{}; // special case root with no children
    }
  if (levels.size () - 1 == level)
    {
      throw std::logic_error{ "level value is to high" };
    }
  auto const &hierarchy = smallMemoryTree.getHierarchy ();
  auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
  auto curentLevelNodeIndexes = smallMemoryTree.getNodeIndexesForLevel (level);
  if (auto curentLevelNodeItr = confu_algorithm::binaryFind (curentLevelNodeIndexes.begin (), curentLevelNodeIndexes.end (), boost::numeric_cast<int64_t> (node)); curentLevelNodeItr != curentLevelNodeIndexes.end ())
    {
      auto currentLevelValuesBeforeNode = std::distance (curentLevelNodeIndexes.begin (), curentLevelNodeItr);
      auto const &childrenOffsetBegin = currentLevelValuesBeforeNode * maxChildren;
      auto childLevelNodeIndexes = smallMemoryTree.getNodeIndexesForLevel (level + 1);
      if (auto childItr = confu_algorithm::binaryFind (childLevelNodeIndexes.begin (), childLevelNodeIndexes.end (), childrenOffsetBegin); childItr != childLevelNodeIndexes.end ())
        {
          auto const hierarchyBegin = boost::numeric_cast<int64_t> ((level == 0) ? 0 : levels.at (level) + *childItr);
          auto const &hierarchyEnd = hierarchyBegin + maxChildren;
          auto result = ResultType{};
          std::get<1> (result) = std::distance (childLevelNodeIndexes.begin (), childItr);
          auto valuesUsed = boost::numeric_cast<int64_t> (smallMemoryTree.getTotalValuesUsedUntilLevel (level + 1)) + std::distance (childLevelNodeIndexes.begin (), childItr);
          for (auto i = hierarchyBegin; i != hierarchyEnd; ++i)
            {
              if (*(hierarchy.begin () + i))
                {
                  std::get<0> (result).push_back (data.at (boost::numeric_cast<uint64_t> (valuesUsed)));
                  valuesUsed++;
                }
            }
          return result;
        }
      else
        {
          return ResultType{};
        }
    }
  else
    {
      throw std::logic_error{ "node has no value" };
    }
}

}

/**
 * @brief This struct is meant to hold all the information of the tree using as less memory as possible
 *
 * @tparam ValueType type of the tree elements
 * @tparam MaxChildrenType type of max children. For example if your biggest node has less than 255 children use uint8_t
 */

template <typename ValueType, typename MaxChildrenType = uint64_t> struct SmallMemoryTreeData
{
  SmallMemoryTreeData (MaxChildrenType maxChildren_, std::vector<bool> hierarchy_, std::vector<ValueType> data_) : maxChildren{ maxChildren_ }, hierarchy{ std::move (hierarchy_) }, data{ std::move (data_) } {};
  SmallMemoryTreeData () = default;
  template <internals::HasIteratorToNode Tree> SmallMemoryTreeData (Tree const &tree) : maxChildren{ boost::numeric_cast<MaxChildrenType> (internals::calculateMaxChildren (tree)) }, hierarchy{ internals::treeHierarchy (tree, maxChildren) }, data{ internals::treeData (tree) } {}

  // clang-format off
    [[nodiscard]]
  auto operator<=> (const SmallMemoryTreeData &) const = default;
  // clang-format on

  MaxChildrenType maxChildren{};
  std::vector<bool> hierarchy{};
  std::vector<ValueType> data{};
};

/**
 * @brief Extends small memory tree data by adding caching variables which are used for value lookup when using childrenByPath function and generate st tree function
 *
 * @tparam ValueType type of the tree elements
 * @tparam MaxChildrenType type of max children. For example if your biggest node has less than 255 children use uint8_t
 * @tparam LevelType lookup to find out which node has which value. saves indicies to denote where a level begins and ends. If you are sure that you know the max tree width you can change the type to a better fitting type and maybe save some memory. I do not think it will lead to savings in storage because databases are good in optimizing this and use only the needed amount of storage
 * @tparam ValuesPerLevelType lookup to find out which node has which value. saves values used until this level. If you are sure that you know the max tree width you can change the type to a better fitting type and maybe save some memory. I do not think it will lead to savings in storage because databases are good in optimizing this and use only the needed amount of storage
 */
template <typename ValueType, typename MaxChildrenType = uint64_t, typename LevelType = uint64_t, typename ValuesPerLevelType = uint64_t> struct SmallMemoryTree
{
public:
  explicit SmallMemoryTree (SmallMemoryTreeData<ValueType, MaxChildrenType> smallMemoryTreeData_) : smallMemoryData{ std::move (smallMemoryTreeData_) }, levels{ internals::calculateLevelSmallMemoryTree<LevelType> (smallMemoryData) }, nodeIndexesPerLevel{ internals::calculateNodeIndexesPerLevel<ValuesPerLevelType> (smallMemoryData.hierarchy, levels) } {}

  // clang-format off
  [[nodiscard]]
  auto operator<=> (const SmallMemoryTree &)  const noexcept = default;
  // clang-format on

  // TODO there should be an option to sort the children so we can use binary search
  [[nodiscard]] SmallMemoryTreeData<ValueType, MaxChildrenType>
  getSmallMemoryData () const &noexcept
  {
    return smallMemoryData;
  }

  [[nodiscard]] std::vector<LevelType> const &
  getLevels () const noexcept
  {
    return levels;
  }

  [[nodiscard]] bool
  getMarkerForEmpty () const noexcept
  {
    return false; // resulting from the way the tree gets saved false is the marker for empty
  }

  [[nodiscard]] MaxChildrenType
  getMaxChildren () const noexcept
  {
    return smallMemoryData.maxChildren;
  }

  [[nodiscard]] std::vector<bool> const &
  getHierarchy () const noexcept
  {
    return smallMemoryData.hierarchy;
  }

  [[nodiscard]] std::vector<ValueType> const &
  getData () const noexcept
  {
    return smallMemoryData.data;
  }

  [[nodiscard]] std::vector<ValuesPerLevelType> const &
  getNodeIndexesForLevel (uint64_t level) const
  {
    if (level >= nodeIndexesPerLevel.size ()) throw std::logic_error{ "level >=nodeIndexesPerLevel.size()" };
    return nodeIndexesPerLevel.at (level);
  }

  [[nodiscard]] ValuesPerLevelType
  getTotalValuesUsedUntilLevel (uint64_t level) const
  {
    if (level >= nodeIndexesPerLevel.size ()) throw std::logic_error{ "level >=nodeIndexesPerLevel.size()" };
    return boost::numeric_cast<ValuesPerLevelType> (std::accumulate (nodeIndexesPerLevel.begin (), nodeIndexesPerLevel.begin () + boost::numeric_cast<int64_t> (level), ValuesPerLevelType{}, [] (auto value, std::vector<ValuesPerLevelType> const &vec) { return boost::numeric_cast<ValuesPerLevelType> (vec.size ()) + value; }));
  }

private:
  SmallMemoryTreeData<ValueType, MaxChildrenType> smallMemoryData{};
  std::vector<LevelType> levels{};
  std::vector<std::vector<ValuesPerLevelType> > nodeIndexesPerLevel{};
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
template <typename ValueType, typename MaxChildrenType, typename LevelType, typename ValuesPerLevelType>
std::optional<std::vector<ValueType> >
childrenByPath (SmallMemoryTree<ValueType, MaxChildrenType, LevelType, ValuesPerLevelType> const &smallMemoryTree, std::vector<ValueType> const &path)
{
  // TODO use std::expected for error handling instead of exception throwing
  if (path.empty ())
    {
      throw std::logic_error{ "path empty" };
    }
  if (smallMemoryTree.getData ().empty ())
    {
      throw std::logic_error{ "no root node" };
    }
  if (smallMemoryTree.getData ().size () == 1 and path.size () and smallMemoryTree.getData ().front () == path.front ())
    {
      return std::get<0> (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, 0, 0));
    }
  if (smallMemoryTree.getData ().size () != 1 and path.size () >= smallMemoryTree.getLevels ().size ())
    {
      throw std::logic_error{ "path is to long" };
    }
  if (smallMemoryTree.getData ().front () != path.front ())
    {
      return {};
    }
  else
    {
      auto node = uint64_t{};
      for (auto i = uint64_t{ 1 }; i < path.size (); ++i)
        {
          auto const &valueToLookFor = path.at (i);
          auto result = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, i - 1, node);
          if (auto itr = std::ranges::find (std::get<0> (result), valueToLookFor); itr != std::get<0> (result).end ())
            {
              auto const usedValuesOfLevelBeforeNode = std::get<1> (result);
              auto const childOffset = std::distance (std::get<0> (result).begin (), itr);
              node = smallMemoryTree.getNodeIndexesForLevel (i).at (boost::numeric_cast<uint64_t> (usedValuesOfLevelBeforeNode + childOffset));
            }
          else
            {
              return {};
            }
        }
      return std::get<0> (internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, path.size () - 1, node));
    }
}
}
