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
#include <ranges>
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
      s << std::endl;
    }
}

template <typename Tree, typename Stream>
void
serialize_indented (const Tree &t, Stream &s, unsigned indent = 2)
{
  serialize_indented (t.df_pre_begin (), t.df_pre_end (), s, indent);
}

template <typename T>
st_tree::tree<T>
generateTree (std::vector<T> const &treeAsVector, auto const &treeLevels)
{
  auto maxChildren = uint64_t{}; // last element is the maxChildren value of the tree
  if constexpr (internals::TupleLike<T>)
    {
      maxChildren = boost::numeric_cast<uint64_t> (std::get<0> (treeAsVector.back ()));
    }
  else
    {
      maxChildren = boost::numeric_cast<uint64_t> (treeAsVector.back ());
    }
  auto treeToFill = st_tree::tree<T>{};
  auto trees = std::__1::deque<st_tree::tree<T> >{};
  for (auto rItr = treeLevels.crbegin (); rItr != treeLevels.crend (); ++rItr)
    {
      if (rItr == treeLevels.crend () - 1)
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
vectorToTree2 (std::vector<T> const &treeAsVector)
{
  auto maxChildren = uint64_t{}; // last element is the maxChildren value of the tree
  if constexpr (internals::TupleLike<T>)
    {
      maxChildren = boost::numeric_cast<uint64_t> (std::get<0> (treeAsVector.back ()));
    }
  else
    {
      maxChildren = boost::numeric_cast<uint64_t> (treeAsVector.back ());
    }
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
  return generateTree (treeAsVector, treeLevels);
}

TEST_CASE ("vectorToTree2 tree to vector")
{
  auto tree = st_tree::tree<uint64_t>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  REQUIRE (vectorToTree2 (treeToVector2 (tree, uint64_t{ 255 })) == tree);
}

TEST_CASE ("vectorToTree2 tree to vector missing element at the end")
{
  auto tree = st_tree::tree<uint64_t>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[1].insert (5);
  tree.root ()[1].insert (6);
  REQUIRE (vectorToTree2 (treeToVector2 (tree, uint64_t{ 255 })) == tree);
}

TEST_CASE ("vectorToTree2 tree to vector besides root all nodes have missing element on the right side")
{
  auto tree = st_tree::tree<uint64_t>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[0].insert (3);
  tree.root ()[0][0].insert (4);
  REQUIRE (vectorToTree2 (treeToVector2 (tree, uint64_t{ 255 })) == tree);
}

TEST_CASE ("vectorToTree2 tree to vector besides root all nodes have missing element on the left side")
{
  auto tree = st_tree::tree<uint64_t>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ()[1].insert (3);
  tree.root ()[1][0].insert (4);
  REQUIRE (vectorToTree2 (treeToVector2 (tree, uint64_t{ 255 })) == tree);
}

TEST_CASE ("vectorToTree2 tree to vector full tree but with 3 children")
{
  auto tree = st_tree::tree<uint64_t>{};
  tree.insert (0);
  tree.root ().insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0].insert (5);
  tree.root ()[0].insert (6);
  tree.root ()[1].insert (7);
  tree.root ()[1].insert (8);
  tree.root ()[1].insert (9);
  tree.root ()[2].insert (10);
  tree.root ()[2].insert (11);
  tree.root ()[2].insert (12);
  REQUIRE (vectorToTree2 (treeToVector2 (tree, uint64_t{ 255 })) == tree);
}

TEST_CASE ("vectorToTree2 tuple")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto myVec = treeToVector2 (tree, std::tuple<uint8_t, int8_t>{ 255, -1 });
  REQUIRE (vectorToTree2 (myVec) == tree);
}