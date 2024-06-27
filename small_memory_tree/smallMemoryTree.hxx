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
#include <expected>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <tuple>
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

template <typename Itr>
auto
dataEndForChildren (Itr begin, Itr end)
{
  // requires a sequence of bool which is partitioned with !(a < b)
  // returns an iterator to the first element which has the value false
  // if there is no true value in the sequence returns begin
  return std::upper_bound (begin, end, false, [] (auto const &a, auto const &b) { return !(a < b); });
}

// TODO try to move code into some functions this is to complicated
[[nodiscard]] auto
childrenAndUsedValuesUntilChildren (auto const &smallMemoryTree, uint64_t level, uint64_t node)
{
  auto const &levels = smallMemoryTree.getLevels ();
  auto const &data = smallMemoryTree.getData ();
  using ChildrenData = std::vector<std::decay_t<decltype (data.front ())> >;
  using ResultType = std::tuple<ChildrenData, int64_t>;
  if (levels.size () == 1 and level == 0 and node == 0)
    {
      return ResultType{}; // special case root with no children
    }
  if (levels.size () - 1 == level)
    {
      throw std::logic_error{ "level value is to high  " };
    }
  auto const &hierarchy = smallMemoryTree.getHierarchy ();
  auto const &maxChildren = boost::numeric_cast<int64_t> (smallMemoryTree.getMaxChildren ());
  auto curentLevelNodeIndexes = smallMemoryTree.getNodeIndexesForLevel (level);
  if (auto curentLevelNodeItr = confu_algorithm::binaryFind (curentLevelNodeIndexes.cbegin (), curentLevelNodeIndexes.cend (), boost::numeric_cast<int64_t> (node)); curentLevelNodeItr != curentLevelNodeIndexes.cend ())
    {
      auto const &currentLevelValuesBeforeNode = std::distance (curentLevelNodeIndexes.cbegin (), curentLevelNodeItr);
      auto const &childrenOffsetBegin = currentLevelValuesBeforeNode * maxChildren;
      auto childLevelNodeIndexes = smallMemoryTree.getNodeIndexesForLevel (level + 1);
      if (auto childItr = confu_algorithm::binaryFind (childLevelNodeIndexes.cbegin (), childLevelNodeIndexes.cend (), childrenOffsetBegin); childItr != childLevelNodeIndexes.cend ())
        {
          auto childrenHierarchyBegin = hierarchy.cbegin () + boost::numeric_cast<int64_t> (levels.at (level) + *childItr);
          auto childrenHierarchyEnd = childrenHierarchyBegin + maxChildren;
          auto const &valuesUsedChildrenLevel = std::distance (childLevelNodeIndexes.cbegin (), childItr);
          auto childrenDataBegin = data.cbegin () + boost::numeric_cast<int64_t> (smallMemoryTree.getTotalValuesUsedUntilLevel (level + 1)) + valuesUsedChildrenLevel;
          auto childrenDataEnd = childrenDataBegin + std::distance (childrenHierarchyBegin, internals::dataEndForChildren (childrenHierarchyBegin, childrenHierarchyEnd));
          return std::make_tuple (ChildrenData{ childrenDataBegin, childrenDataEnd }, valuesUsedChildrenLevel);
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
 * @param smallMemoryTree
 * @param path vector with the values of nodes
 * @return value of the children of the node at the end of the path. Empty vector result means no children. Empty optional means wrong path
 */
template <typename ValueType, typename MaxChildrenType, typename LevelType, typename ValuesPerLevelType>
std::optional<std::vector<ValueType> >
childrenByPath (SmallMemoryTree<ValueType, MaxChildrenType, LevelType, ValuesPerLevelType> const &smallMemoryTree, std::vector<ValueType> const &path, bool childrenAreSorted = false)
{
  // TODO use contracts to check preconditions
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
      for (auto level = uint64_t{ 1 }; level < path.size (); ++level)
        {
          auto const &valueToLookFor = path.at (level);
          auto [childrenData, usedValuesOfLevelBeforeNode] = internals::childrenAndUsedValuesUntilChildren (smallMemoryTree, level - 1, node);
          auto itr = childrenData.begin ();
          if (childrenAreSorted)
            {
              itr = confu_algorithm::binaryFind (childrenData.begin (), childrenData.end (), valueToLookFor);
            }
          else
            {
              itr = std::ranges::find (childrenData, valueToLookFor);
            }
          if (itr != childrenData.end ())
            {
              auto const childOffset = std::distance (childrenData.begin (), itr);
              node = smallMemoryTree.getNodeIndexesForLevel (level).at (boost::numeric_cast<uint64_t> (usedValuesOfLevelBeforeNode + childOffset));
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

namespace small_memory_tree
{
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t> struct Node
{
  // clang-format off
    [[nodiscard]]
  auto operator<=> (const Node &) const = default;
  // clang-format on

  ValueType value{};
  ChildrenOffsetEnd childrenOffsetEnd{};
};

namespace internals
{
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t, HasIteratorToNode TreeAdapter>
[[nodiscard]] std::vector<Node<ValueType, ChildrenOffsetEnd> >
generateNodes (TreeAdapter const &treeAdapter)
{
  std::vector<Node<ValueType, ChildrenOffsetEnd> > results{};
  auto childrenOffsetEnd = uint64_t{};
  std::transform (treeAdapter.constant_breadth_first_traversal_begin (), treeAdapter.constant_breadth_first_traversal_end (), std::back_inserter (results), [&childrenOffsetEnd] (auto const &node) mutable {
    childrenOffsetEnd = childrenOffsetEnd + boost::numeric_cast<uint64_t> (std::distance (node.begin (), node.end ()));
    return Node<ValueType, ChildrenOffsetEnd>{ node.data (), boost::numeric_cast<uint64_t> (childrenOffsetEnd) };
  });
  return results;
}

}
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t> class SmallMemoryTreeNew
{
public:
  SmallMemoryTreeNew () = default;
  template <internals::HasIteratorToNode TreeAdapter> SmallMemoryTreeNew (TreeAdapter const &treeAdapter) : nodes{ small_memory_tree::internals::generateNodes<ValueType> (treeAdapter) } {}

  SmallMemoryTreeNew (std::vector<Node<ValueType, ChildrenOffsetEnd> > nodes_) : nodes{ std::move (nodes_) } {}

  [[nodiscard]] std::vector<Node<ValueType, ChildrenOffsetEnd> > const &
  getNodes () const
  {
    return nodes;
  }

private:
  std::vector<Node<ValueType, ChildrenOffsetEnd> > nodes{};
};

namespace internals
{
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t>
[[nodiscard]] std::expected<ChildrenOffsetEnd, std::string>
calcChildrenCount (SmallMemoryTreeNew<ValueType, ChildrenOffsetEnd> const &smallMemoryTreeNew, uint64_t index)
{
  auto const &nodes = smallMemoryTreeNew.getNodes ();
  if (index >= nodes.size ()) return std::unexpected (std::format ("Index out of bounds nodes.size(): '{}' index '{}'", nodes.size (), index));
  return (index == 0) ? nodes.at (index).childrenOffsetEnd : nodes.at (index).childrenOffsetEnd - nodes.at (index - 1).childrenOffsetEnd;
}

template <typename ValueType, typename ChildrenOffsetEnd = uint64_t>
[[nodiscard]] std::expected<std::tuple<std::vector<Node<ValueType, ChildrenOffsetEnd> >, ChildrenOffsetEnd>, std::string>
calcChildrenWithFirstChildIndex (SmallMemoryTreeNew<ValueType, ChildrenOffsetEnd> const &smallMemoryTreeNew, uint64_t index)
{
  if (auto const &childrenCountExpected = calcChildrenCount (smallMemoryTreeNew, index))
    {
      auto const &nodes = smallMemoryTreeNew.getNodes ();
      auto const &childrenCount = boost::numeric_cast<int64_t> (childrenCountExpected.value ());
      auto const &childrenOffsetEnd = nodes.begin () + boost::numeric_cast<int64_t> (nodes.at (index).childrenOffsetEnd) + 1 /*end has to be one element after the last element*/;
      auto const &childrenBegin = childrenOffsetEnd - childrenCount;
      return std::make_tuple (std::vector<Node<ValueType, ChildrenOffsetEnd> > (childrenBegin, childrenOffsetEnd), std::distance (nodes.begin (), childrenBegin));
    }
  else
    {
      return std::unexpected (childrenCountExpected.error ());
    }
}
}
template <typename ValueType, typename ChildrenOffsetEnd = uint64_t>
[[nodiscard]] std::expected<std::vector<ValueType>, std::string>
calcChildrenForPath (SmallMemoryTreeNew<ValueType, ChildrenOffsetEnd> const &smallMemoryTreeNew, std::vector<ValueType> const &path, bool sortedNodes = false)
{
  if (not path.empty ())
    {
      auto const &nodes = smallMemoryTreeNew.getNodes ();
      auto childrenWithIndexOfFirstChild = std::tuple (std::vector<Node<ValueType, ChildrenOffsetEnd> >{ nodes.front () }, ChildrenOffsetEnd{});
      for (auto i = uint64_t{}; i < path.size (); ++i)
        {
          auto &[nodesToCheck, indexOfFirstChild] = childrenWithIndexOfFirstChild;
          auto const &valueToLookFor = path.at (i);
          if (not nodesToCheck.empty ())
            {
              auto nodeItr = nodesToCheck.begin ();
              if (sortedNodes)
                {
                  nodeItr = confu_algorithm::binaryFind (nodesToCheck.begin (), nodesToCheck.end (), valueToLookFor, {}, &Node<ValueType, ChildrenOffsetEnd>::value);
                }
              else
                {
                  nodeItr = std::ranges::find (nodesToCheck, valueToLookFor, &Node<ValueType, ChildrenOffsetEnd>::value);
                }
              if (nodeItr != nodesToCheck.end ())
                {
                  auto children = internals::calcChildrenWithFirstChildIndex (smallMemoryTreeNew, indexOfFirstChild + boost::numeric_cast<ChildrenOffsetEnd> (std::distance (nodesToCheck.begin (), nodeItr)));
                  if (children)
                    {
                      childrenWithIndexOfFirstChild = std::move (children.value ());
                    }
                  else
                    {
                      return std::unexpected (children.error ());
                    }
                }
              else
                {
                  return std::unexpected (std::format ("invalid path. could not find a match for value with index '{}'.", i));
                }
            }
          else
            {
              return std::unexpected (std::format ("Path too long. Last matching index '{}'.", i - 1));
            }
        }
      auto const &children = std::get<0> (childrenWithIndexOfFirstChild);
      auto result = std::vector<ValueType>{};
      std::ranges::transform (children, std::back_inserter (result), [] (Node<ValueType, ChildrenOffsetEnd> const &node) { return node.value; });
      return result;
    }
  else
    {
      return std::unexpected ("empty path is not allowed");
    }
}
}