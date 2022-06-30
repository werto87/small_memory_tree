#ifndef FF0F9100_DFED_4A55_B6CC_382A1C097294
#define FF0F9100_DFED_4A55_B6CC_382A1C097294

#include "small_memory_tree/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <cstddef>
#include <iterator>
#include <optional>
#include <range/v3/algorithm/find_if.hpp>
#include <type_traits>
#include <vector>

namespace small_memory_tree
{

template <typename T>
long
maxChildren (auto const &treeAsVector, T const &markerForEmpty)
{
  using VectorElementType = typename std::decay<decltype (*treeAsVector.begin ())>::type;
  auto findResult = ranges::find_if (treeAsVector.rbegin (), treeAsVector.rend (), [&markerForEmpty] (VectorElementType const &element) { return element != markerForEmpty; });
  return std::distance (treeAsVector.rbegin (), findResult);
}

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
std::vector<T>
children (std::vector<T> const &vec, size_t index, T markerForEmpty)
{
  auto result = std::vector<T>{};
  for (auto i = size_t{ 1 }; i <= maxChildren (vec, markerForEmpty); i++)
    {
      if (vec[index + i] != markerForEmpty)
        {
          result.push_back (vec[index + i]);
        }
    }
  return result;
}

template <typename T>
std::optional<T>
childWithValue (std::vector<T> const &vec, size_t index, T value, size_t maxChildren)
{
  for (auto i = size_t{ 1 }; i <= boost::numeric_cast<size_t> (maxChildren); i++)
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
indexOfChildrenByPath (std::vector<T> const &vec, std::vector<T> const &path, T const &markerForEmpty)
{
  auto someValue = size_t{ 0 };
  for (auto value : path)
    {
      if (auto index = childWithValue (vec, someValue, value, maxChildren (vec, markerForEmpty)))
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
std::vector<T>
childrenByPath (std::vector<T> const &vec, std::vector<T> const &path, T const &markerForEmpty)
{
  auto result = std::vector<T>{};
  auto tmp = indexOfChildrenByPath (vec, path, markerForEmpty);
  std::transform (tmp.begin (), tmp.end (), std::back_inserter (result), [&vec] (auto const &indexChild) {
    if constexpr (TupleLike<T>)
      {
        return vec[boost::numeric_cast<size_t> (std::get<0> (indexChild))];
      }
    else
      {
        return vec[boost::numeric_cast<size_t> (indexChild)];
      }
  });
  return result;
}
}

#endif /* FF0F9100_DFED_4A55_B6CC_382A1C097294 */
