#pragma once
#include <system_error>

namespace small_memory_tree
{
enum class ApiError
{
  OutOfRange = 1,
  EmptyPath,
  PathTooLong,
  PathDoesNotMatch
};
class ApiErrorCategoryImpl : public std::error_category
{
public:
  char const *
  name () const noexcept override
  {
    return "Small Memory Tree Api Error";
  }
  std::string
  message (int ev) const noexcept override
  {
    switch (static_cast<ApiError> (ev))
      {
      case ApiError::OutOfRange:
        return "Out of Range";
      case ApiError::PathTooLong:
        return "Path is too long";
      case ApiError::PathDoesNotMatch:
        return "Path does not match";
      case ApiError::EmptyPath:
        return "Empty Path is not allowed";
      default:
        return "Unknown Api Error";
      }
  }
};
inline std::error_category const &
ApiErrorCategory ()
{
  static ApiErrorCategoryImpl instance;
  return instance;
}
inline std::error_code
make_error_code (ApiError e)
{
  return std::error_code (static_cast<int> (e), ApiErrorCategory ());
}
inline std::error_condition
make_error_condition (ApiError e)
{
  return std::error_condition (static_cast<int> (e), ApiErrorCategory ());
}
}

namespace std
{
template <> struct is_error_condition_enum<small_memory_tree::ApiError> : public true_type
{
};
}