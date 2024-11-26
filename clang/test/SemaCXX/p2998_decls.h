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

template <typename T> constexpr bool is_array = false;
template <typename T> constexpr bool is_array<T[]> = true;
template <typename T, size_t N> constexpr bool is_array<T[N]> = true;

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

template <typename T, typename U>
concept assignable_from = __is_assignable(T, U); // expected-note {{evaluated to false}}

template <typename T>
concept copy_assignable = __is_assignable(T&, const T&); // expected-note {{evaluated to false}}

namespace ranges {
  namespace _test {
    struct member_begin_end {
      int* begin() const;
      int* end() const;
      friend int* begin(const member_begin_end&);
      friend int* end(const member_begin_end&);
    };
    struct unqualified_begin_end {
      friend int* begin(const unqualified_begin_end&);
      friend int* end(const unqualified_begin_end&);
    };
    struct member_data_size {
      int* data() const;
      size_t size() const;
      friend int* data(const member_data_size&);
      friend size_t size(const member_data_size&);
    };
    struct unqualified_data_size {
      friend int* data(const unqualified_data_size&);
      friend size_t size(const unqualified_data_size&);
    };
  }

  namespace _begin {
    void begin() = delete;
    template <typename T> constexpr bool has_member_begin = requires(T t) { t.begin(); };
    template <typename T> constexpr bool has_unqualified_begin = !has_member_begin<T> && requires(T t) { begin(t); };
    static_assert(has_member_begin<_test::member_begin_end>);
    static_assert(!has_unqualified_begin<_test::member_begin_end>);
    static_assert(!has_member_begin<_test::unqualified_begin_end>);
    static_assert(has_unqualified_begin<_test::unqualified_begin_end>);
    struct impl {
      template <typename R> requires has_member_begin<R>
      static constexpr auto operator()(R&& r) -> decltype(r.begin()) { return r.begin(); }
      template <typename T, size_t N>
      static constexpr T* operator()(T (& a)[N]) noexcept { return a; }
      template <typename R> requires has_unqualified_begin<R>
      static constexpr auto operator()(R&& r) -> decltype(begin(r)) { return begin(r); }
    };
  }

  namespace _end {
    void end() = delete;
    template <typename T> constexpr bool has_member_end = requires(T t) { t.end(); };
    template <typename T> constexpr bool has_unqualified_end = !has_member_end<T> && requires(T t) { end(t); };
    static_assert(has_member_end<_test::member_begin_end>);
    static_assert(!has_unqualified_end<_test::member_begin_end>);
    static_assert(!has_member_end<_test::unqualified_begin_end>);
    static_assert(has_unqualified_end<_test::unqualified_begin_end>);
    struct impl {
      template <typename R> requires has_member_end<R>
      static constexpr auto operator()(R&& r) -> decltype(r.end()) { return r.end(); }
      template <typename T, size_t N>
      static constexpr T* operator()(T (& a)[N]) noexcept { return a + N; }
      template <typename R> requires has_unqualified_end<R>
      static constexpr auto operator()(R&& r) -> decltype(end(r)) { return end(r); }
    };
  }

  namespace _data {
    void data() = delete;
    template <typename T> constexpr bool has_member_data = requires(T t) { { t.data() } -> pointer; };
    template <typename T> constexpr bool has_unqualified_data = !has_member_data<T> && requires(T t) { { data(t) } -> pointer; };
    static_assert(has_member_data<_test::member_data_size>);
    static_assert(!has_unqualified_data<_test::member_data_size>);
    static_assert(!has_member_data<_test::unqualified_data_size>);
    static_assert(has_unqualified_data<_test::unqualified_data_size>);
    struct impl {
      template <typename R> requires has_member_data<R>
      static constexpr auto operator()(R&& r) -> decltype(r.data()) { return r.data(); }
      template <typename T, size_t N>
      static constexpr T* operator()(T (& a)[N]) noexcept { return a; }
      template <typename R> requires has_unqualified_data<R>
      static constexpr auto operator()(R&& r) -> decltype(data(r)) { return data(r); }
    };
  }

  namespace _size {
    void size() = delete;
    template <typename T> constexpr bool has_member_size = requires(T t) { { t.size() } -> convertible_to<size_t>; };
    template <typename T> constexpr bool has_unqualified_size = !has_member_size<T> && requires(T t) { { size(t) } -> convertible_to<size_t>; };
    static_assert(has_member_size<_test::member_data_size>);
    static_assert(!has_unqualified_size<_test::member_data_size>);
    static_assert(!has_member_size<_test::unqualified_data_size>);
    static_assert(has_unqualified_size<_test::unqualified_data_size>);
    struct impl {
      template <typename R> requires has_member_size<R>
      static constexpr auto operator()(const R& r) -> decltype(r.size()) { return r.size(); }
      template <typename T, size_t N>
      static constexpr size_t operator()(T (&)[N]) noexcept { return N; }
      template <typename R> requires has_unqualified_size<R>
      static constexpr auto operator()(const R& r) -> decltype(size(r)) { return size(r); }
    };
  }

  inline constexpr _begin::impl begin;
  inline constexpr _end::impl end;
  inline constexpr _data::impl data;
  inline constexpr _size::impl size;

  static_assert(is_same<decltype(begin(declval<_test::member_begin_end>())), int*>);
  static_assert(is_same<decltype(begin(declval<_test::unqualified_begin_end>())), int*>);
  static_assert(is_same<decltype(end(declval<_test::member_begin_end>())), int*>);
  static_assert(is_same<decltype(end(declval<_test::unqualified_begin_end>())), int*>);
  static_assert(is_same<decltype(data(declval<_test::member_data_size>())), int*>);
  static_assert(is_same<decltype(data(declval<_test::unqualified_data_size>())), int*>);
  static_assert(is_same<decltype(size(declval<_test::member_data_size>())), size_t>);
  static_assert(is_same<decltype(size(declval<_test::unqualified_data_size>())), size_t>);
}

template <typename R>
concept contiguous_range = requires(R r)
{
  { ranges::data(r) } -> pointer;
  { ranges::size(r) } -> convertible_to<size_t>;
};

template <contiguous_range R> using range_reference = decltype(*ranges::data(declval<R>()));
template <contiguous_range R> using range_element = remove_reference<range_reference<R>>;

namespace std {
  template <typename E>
  class initializer_list {
  public:
    using value_type      = E;
    using reference       = const E&;
    using const_reference = const E&;
    using size_type       = size_t;

    using iterator        = const E*;
    using const_iterator  = const E*;

    constexpr initializer_list() noexcept : _p(), _n() {}

    constexpr size_t size() const noexcept    { return _n; }
    constexpr const E* begin() const noexcept { return _p; }
    constexpr const E* end() const noexcept   { return _p + _n; }

    friend const E* data(initializer_list il) noexcept { return il._p; }

  private:
    constexpr initializer_list(const E* p, size_t n) noexcept : _p(p), _n(n) {}

    const E* _p;
    size_type _n;
  };
}
