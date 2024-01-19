/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/treeToVector.hxx"
#include "small_memory_tree/vectorToTree.hxx"
#include <catch2/catch.hpp>
#include <confu_algorithm/createChainViews.hxx>
#include <iostream>
#include <span>
using namespace small_memory_tree;

auto
treeToVector2 (auto const &tree, auto const &markerForEmpty)
{
  using vectorElementType = std::decay_t<decltype (markerForEmpty)>;
  auto const maxChildrenInTree = internals::maxChildren (tree);
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
  result.push_back (boost::numeric_cast<vectorElementType> (maxChildrenInTree));
  return result;
}

template <typename T>
auto
childrenOfRoot (std::vector<T> const &treeAsVector)
{
  //  auto const &maxChildren = treeAsVector.back ();
  //  auto result = std::vector<T>{};
  //  for (auto i = T{ 1 }; i <= maxChildren; ++i)
  //    {
  //      result.push_back (treeAsVector.at (boost::numeric_cast<size_t> (i)));
  //    }
  //  return result;
}

template <typename T>
auto
vectorToTree2 (std::vector<T> const &treeAsVector)
{
  // TODO needs some thinking how to fill the tree
  auto const &markerForEmpty = *(treeAsVector.end () - 2);
  auto const &maxChildren = treeAsVector.back ();
  auto treeToFill = st_tree::tree<T>{};
  treeToFill.insert (treeAsVector.front ());
  //
  treeToFill.root ().insert (treeAsVector.at (1));
  treeToFill.root ().insert (treeAsVector.at (2));
  //
  treeToFill.root ()[0].insert (treeAsVector.at (3));
  // einer ist empty
  //
  treeToFill.root ()[0][0].insert (treeAsVector.at (4));
  treeToFill.root ()[0][0].insert (treeAsVector.at (5));
  //
  // empty
  // empty

  //  auto index = uint64_t{ 1 };
  //  for (auto addedChildren = T{}; addedChildren <= maxChildren; ++addedChildren)
  //    {
  //      ++index;
  //      itr = itr->insert (treeAsVector.at (index));
  //    }
  return treeToFill;
}

TEST_CASE ("vectorToTree2 tree to vector", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0][0].insert (4);
  tree.root ()[0][0].insert (5);
  auto myVec = treeToVector2 (tree, 255);
  auto chainView = confu_algorithm::createChainViewsIncludeBreakingElement (myVec.begin () + 1, myVec.end () - 1, [parentCount = int64_t{ 1 }, maxChildren = myVec.back ()] (auto sequence) mutable {
    if (sequence.size () == parentCount * maxChildren)
      {
        parentCount = std::count_if (sequence.begin (), sequence.end (), [] (auto num) { return num != 255; });
        return false;
      }
    else
      {
        return true;
      }
  });
  childrenOfRoot (myVec);
}