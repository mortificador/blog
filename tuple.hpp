#include <cstddef> // for std::size_t
#include <utility> // for std::foward
#include <string>
#include <type_traits>

// Actual implementation for a type
template <std::size_t _index, typename T>
class _tuple_impl
{
public:
  _tuple_impl(T const &v)
  {
    val = v;
  }

  _tuple_impl(T &&v)
  {
    val = std::move(v);
  }

  T &get()
  {
    return val;
  }

private:
  T val;
};

// general template, will be used only when there is no arguments
template <std::size_t _index, typename... types>
class _tuple_recurr_base
{
};

// This is a partial specialization, so as long as there is at least one argument
// this specialization is preferred to the _tuple_recurr_base<std::size_t, typename ...types>
template <std::size_t _index, typename L, typename... types>
class _tuple_recurr_base<_index, L, types...> : public _tuple_impl<_index, typename std::remove_reference<L>::type>,
                                                public _tuple_recurr_base<_index + 1, types...>
{
public:
  template <typename CL, typename... CArgs>
  _tuple_recurr_base(CL &&arg, CArgs &&... args) : _tuple_impl<_index, typename std::remove_reference<CL>::type>(std::forward<CL>(arg)),
                                                   _tuple_recurr_base<_index + 1, types...>(std::forward<CArgs>(args)...)
  {
  }
};

template <typename L, typename... types>
class tuple : public _tuple_recurr_base<0, L, types...>
{
public:
  // The constructor uses the same recursion as the inheritance
  template <typename... CArgs>
  tuple(CArgs &&... args) : _tuple_recurr_base<0, L, types...>(std::forward<CArgs>(args)...)
  {
  }

  template <typename... Args>
  friend bool operator==(tuple<Args...> &t1, tuple<Args...> &t2);
};

// template deduction guideline
template <typename... CArgs>
tuple(CArgs... args)->tuple<CArgs...>;

// extract_type_at is a class that, given a list of types and an index, defines a type member
// with the type of the index given from the list (zero based index).
// E.g. extract<1, int, double, float>::type == double
// For this we define ::type recursively, until we hit index zero, at that point there is a specialization
// that defines the member ::type, and stops the recursion
template <std::size_t index, typename L, typename... Args>
struct extract_type_at
{
  using type = typename extract_type_at<index - 1, Args...>::type;
};

// This is the stop type. If the index is zero, we define the member type to be the correspondent type
template <typename L, typename... Args>
struct extract_type_at<0, L, Args...>
{
  using type = L;
};

// Method to get the value of a tuple, given an index
// We cast the tuple to the base class that corresponds to the index
// and type for that index
template <std::size_t index, typename... Args>
auto &get(tuple<Args...> &t)
{
  return (static_cast<_tuple_impl<index, typename extract_type_at<index, Args...>::type> &>(t)).get();
}

template <std::size_t index, typename... Args>
bool compare_tuple(tuple<Args...> &t1, tuple<Args...> &t2)
{
  if constexpr (index == 0)
  {
    return get<0>(t1) == get<0>(t2);
  }
  else
  {
    return get<index>(t1) == get<index>(t2) && compare_tuple<index - 1>(t1, t2);
  }
}

template <typename... Args>
bool operator==(tuple<Args...> &t1, tuple<Args...> &t2)
{
  return compare_tuple<sizeof...(Args) - 1>(t1, t2);
}

int main()
{
    std::string c{"lol"};
    int a = 1;
    tuple t{5.0, 6, c};

    return get<1>(t);
}
