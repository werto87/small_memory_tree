#ifndef FF0F9100_DFED_4A55_B6CC_382A1C097294
#define FF0F9100_DFED_4A55_B6CC_382A1C097294

#include "src/util.hxx"
#include <boost/numeric/conversion/cast.hpp>
#include <cstddef>
#include <iterator>
#include <optional>
#include <range/v3/algorithm/find_if.hpp>
#include <type_traits>
#include <vector>
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

#endif /* FF0F9100_DFED_4A55_B6CC_382A1C097294 */
