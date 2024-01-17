#pragma once

/*
Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <tuple>

namespace small_memory_tree::internals
{
// use this to get type at compile time useful to check types in templated code
// WhichType<Something>{};
template <typename...> struct WhichType;

template <typename T> concept TupleLike = requires (T a)
{
  std::tuple_size<T>::value;
  std::get<0> (a);
};
}
