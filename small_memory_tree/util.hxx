#pragma once

#include <tuple>

// use this to get type at compile time useful to check types in templated code
// WhichType<Something>{};
namespace small_memory_tree
{
template <typename...> struct WhichType;

template <typename T> concept TupleLike = requires (T a)
{
  std::tuple_size<T>::value;
  std::get<0> (a);
};
}
