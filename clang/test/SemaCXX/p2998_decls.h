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
