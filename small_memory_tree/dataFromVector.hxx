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
uint64_t
maxChildren (auto const &treeAsVector, T const &markerForEmpty)
{
  using VectorElementType = typename std::decay<decltype (*treeAsVector.begin ())>::type;
  auto findResult = ranges::find_if (treeAsVector.rbegin (), treeAsVector.rend (), [&markerForEmpty] (VectorElementType const &element) { return element != markerForEmpty; });
  return boost::numeric_cast<uint64_t>(std::distance (treeAsVector.rbegin (), findResult));
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

template <typename T>
std::vector<T>
children (std::vector<T> const &vec, uint64_t index, T markerForEmpty)
{
  auto result = std::vector<T>{};
  for (auto i = uint64_t{ 1 }; i <= maxChildren (vec, markerForEmpty); i++)
    {
      if (vec.at(index + i) != markerForEmpty)
        {
          result.push_back (vec.at(index + i));
        }
    }
  return result;
}

template <typename T>
std::optional<uint64_t>
indexOffChildWithValue (std::vector<T> const &vec, uint64_t index, T value, T markerForEmpty)
{
  for (auto i = uint64_t{ 1 }; i <= boost::numeric_cast<uint64_t> (maxChildren (vec, markerForEmpty)); i++)
    {
      if constexpr (TupleLike<T>)
        {
          if (vec.at(boost::numeric_cast<uint64_t> (std::get<0> (vec.at(index + i)))+index + i) == value)
            {
              return boost::numeric_cast<uint64_t> (std::get<0> (vec.at(index + i)))+index + i;
            }
        }
      else
        {
          if (vec.at(boost::numeric_cast<uint64_t> (vec.at(index + i))+index + i) == value)
            {
              return boost::numeric_cast<uint64_t> (vec.at(index + i))+index + i;
            }
        }
    }
  return {};
}




  template <typename T>
  std::vector<T>
  childrenByPath (std::vector<T> const &vec, std::vector<T> const &path, T const &markerForEmpty)
  {
    auto result = std::vector<T>{};
    auto someValue = uint64_t{ 0 };
    for (uint64_t i = 0;i<path.size() ;++i)
      {
        if (auto index = indexOffChildWithValue (vec, someValue, path.at (i), markerForEmpty))
          {
//            if constexpr (TupleLike<T>)
//              {
//                throw std::logic_error{"think about this later"};
                someValue = index.value();
//              }
//            else
//              {
//                throw std::logic_error{"think about this later"};
//                someValue = boost::numeric_cast<uint64_t> (index.value ())+i+someValue+1;
//              }
          }
        else
          {
            return {};
          }
      }
    auto resultChildren=children (vec, someValue, markerForEmpty);
    for(uint64_t i=0;i<resultChildren.size();++i){
        if constexpr (TupleLike<T>)
          {
            result.push_back (vec.at( std::get<0> (resultChildren.at (i)) + someValue + i+1));
          }
        else
          {
            result.push_back (vec.at(resultChildren.at (i) + someValue + i+1));
          }
      }
    return result;
  }

}

#endif /* FF0F9100_DFED_4A55_B6CC_382A1C097294 */
