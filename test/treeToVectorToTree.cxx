#include "src/treeToVector.hxx"
#include "src/vectorToTree.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <catch2/catch.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <range/v3/algorithm.hpp>
#include <range/v3/range_fwd.hpp>
#include <st_tree.h>
#include <sys/types.h>
#include <type_traits>
#include <vector>

template <typename T> concept TupleLike = requires (T a)
{
  std::tuple_size<T>::value;
  std::get<0> (a);
};

template <typename T>
size_t
maxChildren (T const &tree)
{
  auto maxChildren = size_t{};
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
long
maxChildren (auto const &treeAsVector, T const &markerForEmpty)
{
  using VectorElementType = typename std::decay<decltype (*treeAsVector.begin ())>::type;
  auto findResult = ranges::find_if (treeAsVector.rbegin (), treeAsVector.rend (), [&markerForEmpty] (VectorElementType const &element) { return element != markerForEmpty; });
  return std::distance (treeAsVector.rbegin (), findResult);
}

template <typename T>
std::vector<T>
children (std::vector<T> const &vec, size_t index, T emptyMarker)
{
  auto result = std::vector<T>{};
  for (auto i = size_t{ 1 }; i <= boost::numeric_cast<size_t> (maxChildren (vec, emptyMarker)); i++)
    {
      if (vec[index + i] != emptyMarker)
        {
          result.push_back (vec[index + i]);
        }
    }
  return result;
}

template <typename T>
std::optional<T>
childWithValue (std::vector<T> const &vec, size_t index, T markerForEmpty, T value)
{
  for (auto i = size_t{ 1 }; i <= boost::numeric_cast<size_t> (maxChildren (vec, markerForEmpty)); i++)
    {
      if constexpr (TupleLike<T>)
        {
          if (vec[boost::numeric_cast<size_t> (std::get<0> (vec[index + i]))] == value)
            {
              return vec[index + i];
            }
        }
      else
        {
          if (vec[boost::numeric_cast<size_t> (vec[index + i])] == value)
            {
              return vec[index + i];
            }
        }
    }
  return {};
}

template <typename T>
std::vector<T>
childrenByPath (std::vector<T> const &vec, std::vector<T> const &path, T const &markerForEmpty)
{
  auto someValue = size_t{ 0 };
  for (auto value : path)
    {
      if (auto index = childWithValue (vec, someValue, markerForEmpty, value))
        {
          if constexpr (TupleLike<T>)
            {
              someValue = boost::numeric_cast<size_t> (std::get<0> (index.value ()));
            }
          else
            {
              someValue = boost::numeric_cast<size_t> (index.value ());
            }
        }
      else
        {
          return {};
        }
    }
  return children (vec, someValue, markerForEmpty);
}

template <typename T>
void
fillChildren (std::vector<T> &vec, size_t maxChildren, T const &markerForChild)
{
  auto nodeCount = size_t{ 1 };
  for (auto &value : vec)
    {
      if (value == markerForChild)
        {
          if constexpr (TupleLike<T>)
            {
              std::get<0> (value) = static_cast<typename std::decay<decltype (std::get<0> (value))>::type> (nodeCount * (maxChildren + 1));
              nodeCount++;
            }
          else
            {
              value = static_cast<T> (nodeCount * (maxChildren + 1));
              nodeCount++;
            }
        }
    }
}

template <typename T>
std::vector<T>
treeToVector (auto const &tree, T const &markerForEmpty, T const &markerForChild, std::function<typename std::decay<decltype (markerForEmpty)>::type (typename std::decay<decltype (*tree.begin ())>::type const &node)> nodeToData = {})
{
  auto const maxChildrenInTree = maxChildren (tree);
  auto result = std::vector<T>{};
  for (auto &node : tree)
    {
      if (nodeToData)
        {
          result.push_back (nodeToData (node));
        }
      else
        {
          if constexpr (std::is_same<typename std::decay<decltype (node.data ())>::type, T>::value)
            {
              result.push_back (node.data ());
            }
        }
      auto currentChildren = size_t{};
      while (currentChildren < node.size ())
        {
          result.push_back (markerForChild);
          currentChildren++;
        }
      while (currentChildren < maxChildrenInTree)
        {
          result.push_back (markerForEmpty);
          currentChildren++;
        }
    }
  result.shrink_to_fit ();
  fillChildren (result, maxChildrenInTree, markerForChild);
  return result;
}

template <typename T>
void
addChildren (auto const &treeAsVector, T const &value, auto &treeItr, auto &markerForEmpty)
{
  auto parentIndex = size_t{};
  if constexpr (TupleLike<T>)
    {
      parentIndex = std::get<0> (value);
    }
  else
    {
      parentIndex = boost::numeric_cast<size_t> (value);
    }
  for (auto const &child : children (treeAsVector, parentIndex, markerForEmpty))
    {
      auto childIndex = size_t{};
      if constexpr (TupleLike<T>)
        {
          childIndex = std::get<0> (child);
        }
      else
        {
          childIndex = boost::numeric_cast<size_t> (child);
        }
      auto parentItr = treeItr->insert (treeAsVector[childIndex]);
      addChildren (treeAsVector, child, parentItr, markerForEmpty);
    }
}
template <typename T>
void
fillTree (auto const &treeAsVector, auto &tree, T &markerForEmpty)
{
  for (auto const &child : children (treeAsVector, 0, markerForEmpty))
    {
      auto childIndex = size_t{};
      if constexpr (TupleLike<T>)
        {
          childIndex = std::get<0> (child);
        }
      else
        {
          childIndex = boost::numeric_cast<size_t> (child);
        }
      auto parentItr = tree.root ().insert (treeAsVector[childIndex]);
      addChildren (treeAsVector, child, parentItr, markerForEmpty);
    }
}

// TODO maybe instead of index use relative position index is limited to the type size. relative position is kinda limited to type size but not that strong
// index in unsigned char max allowed size of vector is 253 with relative position max distance between parent and the furthest child smaller 253.
template <typename T>
auto
vectorToTree (auto const &treeAsVector, T const &markerForEmpty)
{
  auto result = st_tree::tree<T>{};
  result.insert (treeAsVector.at (0));
  fillTree (treeAsVector, result, markerForEmpty);
  return result;
}

TEST_CASE ("2 children", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  auto myVec = treeToVector (tree, 255, 254);
  for (auto &value : childrenByPath (myVec, { 2, 4 }, 255))
    {
      REQUIRE (myVec[boost::numeric_cast<size_t> (value)] == 42);
    }
}

TEST_CASE ("maxChildren", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  REQUIRE (maxChildren (treeToVector (tree, 255, 254), 255) == 2);
}

TEST_CASE ("vectorToTree tree to vector", "[abc]")
{
  auto tree = st_tree::tree<int>{};
  tree.insert (1);
  tree.root ().insert (2);
  tree.root ().insert (3);
  tree.root ()[0].insert (4);
  tree.root ()[0][0].insert (42);
  REQUIRE (vectorToTree (treeToVector (tree, 255, 254), 255) == tree);
}

TEST_CASE ("3 children and tuple", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  auto myVec = treeToVector (tree, std::tuple<uint8_t, int8_t>{ 255, -1 }, std::tuple<uint8_t, int8_t>{ 254, -1 });
  for (auto &value : childrenByPath (myVec, { { 2, 2 }, { 4, 4 } }, { 255, -1 }))
    {
      REQUIRE (myVec[boost::numeric_cast<size_t> (std::get<0> (value))] == std::tuple<uint8_t, int8_t>{ 42, 42 });
    }
}

TEST_CASE ("3 children and tuple vectorToTree tree to vector", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<uint8_t, int8_t> >{};
  tree.insert ({ 1, 1 });
  tree.root ().insert ({ 2, 2 });
  tree.root ().insert ({ 3, 3 });
  tree.root ().insert ({ 69, 69 });
  tree.root ()[0].insert ({ 4, 4 });
  tree.root ()[0][0].insert ({ 42, 42 });
  // REQUIRE (vectorToTree (treeToVector (tree, std::tuple<uint8_t, int8_t>{ 255, -1 }, std::tuple<uint8_t, int8_t>{ 254, -1 }), std::tuple<uint8_t, int8_t>{ 255, -1 }) == tree);
}

enum class Result : uint8_t
{
  Undefined,
  DefendWon,
  Draw,
  AttackWon
};

class Action
{
public:
  Action () = default;
  Action (std::uint8_t cardPlayed_) : cardPlayed (cardPlayed_) {}
  auto operator<=> (const Action &) const = default;

  std::uint8_t
  value () const
  {
    return cardPlayed;
  };

private:
  std::uint8_t cardPlayed{ 253 };
};

TEST_CASE ("treeToVector", "[abc]")
{
  auto tree = st_tree::tree<std::tuple<Result, bool>, st_tree::keyed<Action> >{};
  tree.insert ({ Result::Undefined, true });
  tree.root ().insert (0, { Result::Undefined, true });
  tree.root ().insert (2, { Result::Undefined, true });
  tree.root ().insert (3, { Result::Undefined, true });
  tree.root ()[0].insert (0, { Result::Undefined, true });
  tree.root ()[0][0].insert (1, { Result::Undefined, true });
  auto myVec = treeToVector (tree, std::tuple<uint8_t, Result>{ 255, Result::Undefined }, std::tuple<uint8_t, Result>{ 254, Result::Undefined }, [] (auto const &node) { return std::tuple<uint8_t, Result>{ node.key ().value (), std::get<0> (node.data ()) }; });
  REQUIRE (myVec.size () == 24);
  auto result = childrenByPath (myVec, { { 0, Result::Undefined }, { 0, Result::Undefined } }, { 255, Result::Undefined });
  REQUIRE_FALSE (result.empty ());
  REQUIRE (myVec[std::get<0> (result.at (0))] == std::tuple<uint8_t, Result>{ 1, Result::Undefined });
}