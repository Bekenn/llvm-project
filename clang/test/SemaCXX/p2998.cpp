// RUN: %clang_cc1 -std=c++2c -verify %s
// expected-no-diagnostics

using size_t = decltype(sizeof(0));

template <typename T> T&& declval() noexcept;

template <auto V>
struct constant
{
  static constexpr bool value = V;
  constexpr operator bool() const noexcept { return value; }
};

using false_type = constant<false>;
using true_type = constant<true>;

template <typename T> constexpr bool is_pointer = false;
template <typename T> constexpr bool is_pointer<T*> = true;
template <typename T> concept pointer = is_pointer<T>;

template <typename T, typename U> constexpr bool is_same = false;
template <typename T> constexpr bool is_same<T, T> = true;

template <typename T> extern T _remove_reference;
template <typename T> extern T _remove_reference<T&>;
template <typename T> extern T _remove_reference<T&&>;
template <typename T> using remove_reference = decltype(_remove_reference<T>);

template <typename T> extern T _remove_pointer;
template <typename T> extern T _remove_pointer<T*>;
template <typename T> using remove_pointer = decltype(_remove_pointer<T>);

template <typename T> using add_pointer = remove_reference<T>*;

template <typename T, typename U>
concept convertible_to = __is_convertible(T, U);

template <typename R>
concept contiguous_range = requires(R r)
{
  { r.data() } -> pointer;
  { r.size() } -> convertible_to<size_t>;
};

template <contiguous_range R> using range_reference = decltype(*declval<R>().data());
template <contiguous_range R> using range_element = remove_reference<range_reference<R>>;

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
    requires __is_convertible(range_element<R> (*)[], T (*)[])
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

template <contiguous_range R>
span(R&&) -> span<range_element<R>>;

template <typename T>
using const_span = span<const T>;

// User-declared deduction guides for alias templates
template <typename T> const_span(span<T>) -> const_span<T>;
template <contiguous_range R> const_span(R&&) -> const_span<range_element<R>>;

int x[5];
span s = x; static_assert(is_same<decltype(s), span<int>>);
const_span cs = s; static_assert(is_same<decltype(cs), span<const int>>);
const_span cx = x; static_assert(is_same<decltype(cx), span<const int>>);

template <typename T>
struct empty_range
{
  constexpr T* data() noexcept { return nullptr; }
  constexpr size_t size() noexcept { return 0; }
};

span sn = empty_range<float>(); static_assert(is_same<decltype(sn), span<float>>);
const_span csn = empty_range<float>(); static_assert(is_same<decltype(csn), span<const float>>);
