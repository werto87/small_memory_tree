// use this to get type at compile time useful to check types in templated code
// WhichType<Something>{};
template <typename...> struct WhichType;
