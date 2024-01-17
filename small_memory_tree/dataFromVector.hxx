#pragma once

#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <optional>
#include <vector>

namespace small_memory_tree
{

namespace internals{

template <typename T>
uint64_t
maxChildren (std::vector<T> const &treeAsVector)
{
  using VectorElementType = typename std::decay<decltype (*treeAsVector.begin ())>::type;
  auto findResult = std::ranges::find_if (treeAsVector.rbegin (), treeAsVector.rend (), [markerForEmpty=treeAsVector.back ()] (VectorElementType const &element) { return element != markerForEmpty; });
  return boost::numeric_cast<uint64_t> (std::distance (treeAsVector.rbegin (), findResult));
}

template <typename T>
uint64_t
maxChildren (T const &tree)
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

}
/**
 * children of a node using vector as a tree
 * @param vec vector in tree form
 * @param index node to get children from
 * @return the children of the node
 */
template <typename T>
std::vector<T>
children (std::vector<T> const &vec, uint64_t index)
{
  auto const &markerForEmpty = vec.back ();
  auto result = std::vector<T>{};
  for (auto i = uint64_t{ 1 }; i <= internals::maxChildren (vec); i++)
    {
      if (vec.at(index + i) != markerForEmpty)
        {
          result.push_back (vec.at(index + i));
        }
    }
  return result;
}
/**
 * nodes can have multiple children. If one of the node matches the given value return the index of the node
 * @param vec vector in tree form
 * @param index node to search children
 * @param valueToLookFor valueToLookFor value to look for
 * @return index of first child which has the same value as valueToLookFor
 */
template <typename T>
std::optional<uint64_t>
indexOffChildWithValue (std::vector<T> const &vec, uint64_t index, T valueToLookFor)
{
  for (auto i = uint64_t{ 1 }; i <= boost::numeric_cast<uint64_t> (internals::maxChildren (vec)); i++)
    {
      if constexpr (internals::TupleLike<T>)
        {
          if (vec.at(boost::numeric_cast<uint64_t> (std::get<0> (vec.at(index + i)))+index + i) == valueToLookFor)
            {
              return boost::numeric_cast<uint64_t> (std::get<0> (vec.at(index + i)))+index + i;
            }
        }
      else
        {
          if (vec.at(boost::numeric_cast<uint64_t> (vec.at(index + i))+index + i) == valueToLookFor)
            {
              return boost::numeric_cast<uint64_t> (vec.at(index + i))+index + i;
            }
        }
    }
  return {};
}



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
 * @param vec vector in tree form
 * @param path vector with the values of nodes
 * @return value of the children of the node at the end of the path
 */
  template <typename T>
  std::vector<T>
  childrenByPath (std::vector<T> const &vec, std::vector<T> const &path)
  {
    auto result = std::vector<T>{};
    auto index = uint64_t{ 0 };
    for (uint64_t i = 0;i<path.size() ;++i)
      {
        if (auto indexOptional = indexOffChildWithValue (vec, index, path.at (i)))
          {
            index = indexOptional.value();
          }
        else
          {
            return {};
          }
      }
    auto resultChildren=children (vec, index);
    for(uint64_t i=0;i<resultChildren.size();++i){
        if constexpr (internals::TupleLike<T>)
          {
            result.push_back (vec.at( boost::numeric_cast<uint64_t>(std::get<0> (resultChildren.at (i))) + index + i+1));
          }
        else
          {
            result.push_back (vec.at(boost::numeric_cast<uint64_t>(resultChildren.at (i)) + index + i+1));
          }
      }
    return result;
  }
}
