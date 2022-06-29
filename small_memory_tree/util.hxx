

#ifndef CF0ED571_02E1_4B31_B00A_BF110CD2C982
#define CF0ED571_02E1_4B31_B00A_BF110CD2C982

#include <tuple>

// use this to get type at compile time useful to check types in templated code
// WhichType<Something>{};

template <typename...> struct WhichType;

template <typename T> concept TupleLike = requires (T a)
{
  std::tuple_size<T>::value;
  std::get<0> (a);
};

#endif /* CF0ED571_02E1_4B31_B00A_BF110CD2C982 */
