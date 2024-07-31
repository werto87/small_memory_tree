#pragma once
#include <system_error>

namespace small_memory_tree
{
enum class ApiError
{
  OutOfRange = 1,
  PathTooLong,
  WrongPath
};
class ApiErrorCategoryImpl : public std::error_category
{
public:
  const char *
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
        return "Out of Range Error";
      case ApiError::PathTooLong:
        return "Path is too long Error";
      case ApiError::WrongPath:
        return "Path is Wrong Error";
      default:
        return "Unknown Api Error";
      }
  }
};

const std::error_category &
ApiErrorCategory ()
{
  static ApiErrorCategoryImpl instance;
  return instance;
}
std::error_code
make_error_code (ApiError e)
{
  return std::error_code (static_cast<int> (e), ApiErrorCategory ());
}
std::error_condition
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