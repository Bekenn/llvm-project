// RUN: %clang_cc1 -std=c++2c -verify %s -DUSE_ALIAS_GUIDES=1
// RUN: %clang_cc1 -std=c++2c -verify %s -DUSE_MULTI_RETURN=1
// RUN: %clang_cc1 -std=c++2c -verify %s -DUSE_CONSTRAINTS=1
// expected-no-diagnostics

#include "p2998_decls.h"

template <typename T>
class span
{
public:
  constexpr span() noexcept : _p(nullptr), _n(0) {}
  constexpr span(T* p, size_t n) noexcept : _p(p), _n(n) {}
  constexpr span(T* f, T* l) noexcept : _p(f), _n(size_t(l - f)) {}
  template <size_t N>
    constexpr span(T (& a)[N]) noexcept : _p(a), _n(N) {}
  template <contiguous_range R>
    requires convertible_to<range_element<R> (*)[], T (*)[]>
      constexpr span(R&& r) : _p(r.data()), _n(r.size()) {}

public:
  constexpr T* begin() const noexcept { return _p; }
  constexpr T* end() const noexcept { return _p + _n; }
  constexpr T* data() const noexcept { return _p; }
  constexpr size_t size() const noexcept { return _n; }

private:
  T* _p;
  size_t _n;
};

template <typename T, size_t N>
span(T (&)[N]) -> span<T>;

#if USE_ALIAS_GUIDES

span() -> span<int>;

template <contiguous_range R>
span(R&&) -> span<range_element<R>>;

#elif USE_MULTI_RETURN

span()
  -> span<int>,
     span<const int>,
     span<volatile int>,
     span<const volatile int>;

template <typename T>
span(span<T>)
  -> span<T>,
     span<const T>,
     span<volatile T>,
     span<const volatile T>;

template <contiguous_range R>
span(R&&)
  -> span<range_element<R>>,
     span<const range_element<R>>,
     span<volatile range_element<R>>,
     span<const volatile range_element<R>>;

#elif USE_CONSTRAINTS

// Not a template, no way to constrain overloads
span() -> span<int>;

template <typename T> requires true
  span(span<T>) -> span<T>;
template <typename T>
  span(span<T>) ->span<const T>;
template <typename T>
  span(span<T>) ->span<volatile T>;
// template <typename T>
//   span(span<T>) ->span<const volatile T>;

template <contiguous_range R> requires true
  span(R&&) -> span<range_element<R>>;
template <contiguous_range R>
  span(R&&) -> span<const range_element<R>>;
template <contiguous_range R>
  span(R&&) -> span<volatile range_element<R>>;
// template <contiguous_range R>
//   span(R&&) -> span<const volatile range_element<R>>;

#endif

template <typename T>
using const_span = span<const T>;
template <typename T>
using volatile_span = span<volatile T>;
template <typename T>
using const_volatile_span = const_span<volatile T>;

#if USE_ALIAS_GUIDES
// User-declared deduction guides for alias templates
template <typename T>
  const_span(span<T>) -> const_span<T>;
template <contiguous_range R>
  const_span(R&&) -> const_span<range_element<R>>;

template <typename T>
  volatile_span(span<T>) -> volatile_span<T>;
template <contiguous_range R>
  volatile_span(R&&) -> volatile_span<range_element<R>>;

template <typename T>
  const_volatile_span(span<T>) -> const_volatile_span<T>;
template <contiguous_range R>
  const_volatile_span(R&&) -> const_volatile_span<range_element<R>>;
#endif

span def_s; static_assert(is_same<decltype(def_s), span<int>>);
#if USE_MULTI_RETURN && CLANG_BUGFIX
const_span def_cs; static_assert(is_same<decltype(def_cs), span<const int>>);
volatile_span def_vs; static_assert(is_same<decltype(def_vs), span<volatile int>>);
const_volatile_span def_cvs; static_assert(is_same<decltype(def_cvs), span<const volatile int>>);
#endif

int x[5];
span s = x; static_assert(is_same<decltype(s), span<int>>);
const_span cs = s; static_assert(is_same<decltype(cs), span<const int>>);
const_span cx = x; static_assert(is_same<decltype(cx), span<const int>>);
volatile_span vs = s; static_assert(is_same<decltype(vs), span<volatile int>>);
#if !USE_CONSTRAINTS
const_volatile_span cvs = s; static_assert(is_same<decltype(cvs), span<const volatile int>>);
#endif

template <typename T>
struct empty_range
{
  constexpr T* data() noexcept { return nullptr; }
  constexpr size_t size() noexcept { return 0; }
};

span sn = empty_range<float>(); static_assert(is_same<decltype(sn), span<float>>);
const_span csn = empty_range<float>(); static_assert(is_same<decltype(csn), span<const float>>);
volatile_span vsn = empty_range<float>(); static_assert(is_same<decltype(vsn), span<volatile float>>);
#if !USE_CONSTRAINTS
const_volatile_span cvsn = empty_range<float>(); static_assert(is_same<decltype(cvsn), span<const volatile float>>);
#endif
