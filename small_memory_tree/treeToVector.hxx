#pragma once

#include "small_memory_tree/dataFromVector.hxx"
#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <stdexcept>

namespace small_memory_tree
{

namespace internals
{
template <typename T>
void
fillChildren (std::vector<T> &vec, uint64_t maxChildren, T const &markerForChild)
{
  auto nodeCount = uint64_t{ 1 };
  for (uint64_t i = 0; i < vec.size (); ++i)
    {
      auto &value = vec.at (i);
      if (value == markerForChild)
        {
          if constexpr (TupleLike<T>)
            {
              try
                {
                  std::get<0> (value) = boost::numeric_cast<typename std::decay<decltype (std::get<0> (value))>::type> (nodeCount * (maxChildren + 1) - i);
                }
              catch (std::bad_cast &)
                {
                  throw std::logic_error ("the distance from the child to its data does not fit in the datatype consider to use a bigger datatype or decrease the max children of the tree. The value is '" + std::string{ std::to_string (nodeCount * (maxChildren + 1) - i) } + "' the max value is '" + std::string{ std::to_string (std::numeric_limits<typename std::decay<decltype (std::get<0> (value))>::type>::max ()) } + "'");
                }
              nodeCount++;
            }
          else
            {
              try
                {
                  value = boost::numeric_cast<T> ((nodeCount * (maxChildren + 1)) - i);
                }
              catch (std::bad_cast &)
                {
                  throw std::logic_error ("the distance from the child to its data does not fit in the datatype consider to use a bigger datatype or decrease the max children of the tree. The value is '" + std::string{ std::to_string (nodeCount * (maxChildren + 1) - i) } + "' the max value is '" + std::string{ std::to_string (std::numeric_limits<T>::max ()) } + "'");
                }
              nodeCount++;
            }
          auto const &markerForEmpty = vec.back ();
          if (value == markerForEmpty)
            {
              throw std::logic_error ("the distance from the child to its data is the same as the value of 'marker for empty'. This leads to an ambiguity. Increase the value of 'marker for empty'. If it is already the max value of the datatype consider to use a bigger datatype or decrease the max children of the tree.");
            }
        }
    }
}

template <typename T>
void
validateData (T const &data, T const &markerForEmpty, T const &markerForChild)
{
  if (data == markerForEmpty)
    {
      throw std::logic_error{ "data == markerForEmpty. Please check your data it is not allowed to have data with the same value as markerForEmpty" };
    }
  else if (data == markerForChild)
    {
      throw std::logic_error{ "data == markerForChild. Please check your data it is not allowed to have data with the same value as markerForChild" };
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
          auto data = nodeToData (node);
          internals::validateData (data, markerForEmpty, markerForChild);
          result.push_back (data);
        }
      else
        {
          if constexpr (std::is_same<typename std::decay<decltype (node.data ())>::type, T>::value)
            {
              auto data = node.data ();
              internals::validateData (data, markerForEmpty, markerForChild);
              result.push_back (node.data ());
            }
          else
            {
              throw std::logic_error{ "node type and element type of result vector is not the same." };
            }
        }
      auto currentChildren = uint64_t{};
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
  internals::fillChildren (result, maxChildrenInTree, markerForChild);
  return result;
}
}
