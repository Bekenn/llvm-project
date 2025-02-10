#include <initializer_list>

#include <array>
#include <concepts>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <type_traits>

#include <cassert>
#include <cstddef>

namespace p2998 {
  // constants
  inline constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

  namespace detail {
    template <typename T>
      concept integral_constant_like =
        std::is_integral_v<decltype(T::value)> &&
        !std::is_same_v<bool, std::remove_const_t<decltype(T::value)>> &&
        std::convertible_to<T, decltype(T::value)> &&
        std::equality_comparable_with<T, decltype(T::value)> &&
        std::bool_constant<T() == T::value>::value &&
        std::bool_constant<static_cast<decltype(T::value)>(T()) == T::value>::value;

    template <typename T>
      constexpr size_t maybe_static_ext = dynamic_extent;
    template <integral_constant_like T>
      constexpr size_t maybe_static_ext<T> = {T::value};
  }

  // [views.span], class template span
  template <typename ElementType, size_t Extent = dynamic_extent>
    class span;
}

template <typename ElementType, size_t Extent>
  constexpr bool std::ranges::enable_view<p2998::span<ElementType, Extent>> = true;
template <typename ElementType, size_t Extent>
  constexpr bool std::ranges::enable_borrowed_range<p2998::span<ElementType, Extent>> = true;

namespace p2998 {
  template <typename T> inline constexpr bool _is_span = false;
  template <typename T> inline constexpr bool _is_span<span<T>> = true;

  template <typename T> inline constexpr bool _is_array = false;
  template <typename T, size_t N> inline constexpr bool _is_array<std::array<T, N>> = true;

  template <typename ElementType, size_t Extent>
  class _span_base {
  public:
    constexpr _span_base() noexcept requires(Extent == 0) : data_() {}
    explicit constexpr _span_base(ElementType* data, size_t size) noexcept : data_(data) { assert(size == Extent); }

    constexpr size_t size() const noexcept { return Extent; }

  protected:
    ElementType* data_;
  };

  template <typename ElementType>
  class _span_base<ElementType, dynamic_extent> {
  public:
    constexpr _span_base() noexcept : data_(), size_() {}
    explicit constexpr _span_base(ElementType* data, size_t size) noexcept : data_(data), size_(size) {}

    constexpr size_t size() const noexcept { return size_; }

  protected:
    ElementType* data_;
    size_t size_;
  };

  template <typename ElementType, size_t Extent>
  class span : _span_base<ElementType, Extent> {
  public:
    // constants and types
    using element_type = ElementType;
    using value_type = std::remove_cv_t<ElementType>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<pointer>;
    using const_reverse_iterator = std::reverse_iterator<const_pointer>;
    static constexpr size_type extent = Extent;

    // [span.cons], constructors, copy, and assignment
    constexpr span() noexcept = default;
    template <std::contiguous_iterator It>
      requires std::is_convertible_v<std::remove_reference_t<std::iter_reference_t<It>> (*)[], element_type (*)[]>
        constexpr explicit(extent != dynamic_extent) span(It first, size_type count)
          : span::_span_base(std::to_address(first), count) {}
    template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
      requires(!std::is_convertible_v<It, size_t>)
        constexpr explicit(extent != dynamic_extent) span(It first, End last)
          : span::_span_base(std::to_address(first), size_type(last - first)) {}
    template <size_t N>
      requires(extent == dynamic_extent || N == extent)
        constexpr span(std::type_identity_t<element_type> (&arr)[N]) noexcept
          : span::_span_base(arr, N) {}
    template <typename T, size_t N>
      requires((extent == dynamic_extent || N == extent) && std::is_convertible_v<T (*)[], element_type (*)[]>)
        constexpr span(std::array<T, N>& arr) noexcept
          : span::_span_base(arr.data(), N) {}
    template <typename T, size_t N>
      requires((extent == dynamic_extent || N == extent) && std::is_convertible_v<const T (*)[], element_type (*)[]>)
        constexpr span(const std::array<T, N>& arr) noexcept
          : span::_span_base(arr.data(), N) {}
    template <std::ranges::contiguous_range R>
      requires(std::ranges::sized_range<R>
            && (std::is_const_v<element_type> || std::ranges::borrowed_range<R>)
            && !_is_span<std::remove_cvref_t<R>>
            && !_is_array<std::remove_cvref_t<R>>
            && !std::is_array_v<std::remove_cvref_t<R>>
            && !std::is_convertible_v<std::remove_reference_t<std::ranges::range_reference_t<R>> (*)[], element_type (*)[]>)
        constexpr explicit(extent != dynamic_extent) span(R&& r)
          : span::_span_base(std::ranges::data(r), std::ranges::size(r)) {}
    constexpr explicit(extent != dynamic_extent) span(std::initializer_list<value_type> il)
      requires std::is_const_v<element_type>
        : span::_span_base(il.begin(), il.size()) {}
    constexpr span(const span& other) noexcept = default;
    template <typename OtherElementType, size_t OtherExtent>
      requires((extent == dynamic_extent || OtherExtent == dynamic_extent || extent == OtherExtent)
            && std::is_convertible_v<OtherElementType (*)[], element_type (*)[]>)
        constexpr explicit(extent != dynamic_extent && OtherExtent == dynamic_extent) span(const span<OtherElementType, OtherExtent>& s) noexcept
          : span::_span_base(s.data(), s.size()) {}

    constexpr span& operator=(const span& other) noexcept = default;

    // [span.sub], subviews
    template <size_t Count>
      constexpr span<element_type, Count> first() const
        { static_assert(Count <= Extent); assert(Count <= size()); return { this->data_, Count }; }
    template <size_t Count>
      constexpr span<element_type, Count> last() const
        { static_assert(Count <= Extent); assert(Count <= size()); return { this->data_ + (size() - Count), Count }; }
    template <size_t Offset, size_t Count = dynamic_extent>
      constexpr span<element_type, Count != dynamic_extent ? Count : Extent != dynamic_extent ? Extent - Offset : dynamic_extent> subspan() const {
        static_assert(Offset <= Extent);
        static_assert(Count == dynamic_extent || Count <= Extent - Offset);
        assert(Offset <= size());
        assert(Count == dynamic_extent || Count <= size() - Offset);
        return { this->data_ + Offset, Count != dynamic_extent ? Count : size() - Offset };
      }

    constexpr span<element_type, dynamic_extent> first(size_type count) const
      { assert(count <= size()); return { this->data_, count }; }
    constexpr span<element_type, dynamic_extent> last(size_type count) const
      { assert(count <= size()); return { this->data_ + (size() - count), count }; }
    constexpr span<element_type, dynamic_extent> subspan(size_type offset, size_type count = dynamic_extent) const {
      assert(offset <= size());
      assert(count == dynamic_extent || count <= size() - offset);
      return { this->data_ + offset, count == dynamic_extent ? size() - offset : count };
    }

    // [span.obs], observers
    using span::_span_base::size;
    constexpr bool empty() const noexcept { return size() == 0; }

    // [span.elem], element access
    constexpr reference operator[](size_type idx) const { assert(idx < size()); return this->data_[idx]; }
    constexpr reference at(size_type idx) const
      { if (idx >= size()) throw std::out_of_range("span index out of range"); return this->data_[idx]; }
    constexpr reference front() const { assert(size() != 0); return this->data_[0]; }
    constexpr reference back() const { assert(size() != 0); return this->data_[size() - 1]; }
    constexpr pointer data() const noexcept { return this->data_; }

    // [span.iterators], iterator support
    constexpr iterator begin() const noexcept { return this->data_; }
    constexpr iterator end() const noexcept { return this->data_ + size(); }
    constexpr const_iterator cbegin() const noexcept { return this->data_; }
    constexpr const_iterator cend() const noexcept { return this->data_ + size(); }
    constexpr reverse_iterator rbegin() const noexcept { return std::reverse_iterator(this->data_ + size()); }
    constexpr reverse_iterator rend() const noexcept { return std::reverse_iterator(this->data_); }
    constexpr const_reverse_iterator crbegin() const noexcept { return std::reverse_iterator(this->data_ + size()); }
    constexpr const_reverse_iterator crend() const noexcept { return std::reverse_iterator(this->data_); }
  };

  template <std::contiguous_iterator It, typename EndOrSize>
    span(It, EndOrSize) -> span<std::remove_reference_t<std::iter_reference_t<It>>, detail::maybe_static_ext<EndOrSize>>;
  template <typename T, size_t N>
    span(T (&)[N])
      -> span<T, N>,
         span<const T, N>;
  template <typename T, size_t N>
    span(std::array<T, N>&) -> span<T, N>;
  template <typename T, size_t N>
    span(const std::array<T, N>&) -> span<const T, N>;
  template <typename R>
    span(R&&)
      -> span<std::remove_reference_t<std::ranges::range_reference_t<R>>>,
         span<const std::remove_reference_t<std::ranges::range_reference_t<R>>>;
}

template <typename T>
  void f(p2998::span<T>);
template <typename T>
  void g(p2998::span<const T>);
template <typename T, std::size_t Extent = p2998::dynamic_extent>
  void h(p2998::span<const T, Extent>);

template <typename T>
  using dynamic_span = p2998::span<T>;
template <typename T>
  using const_dynamic_span = p2998::span<const T>;
template <typename T, std::size_t Extent = p2998::dynamic_extent>
  using const_span = p2998::span<const T, Extent>;

void example()
{
  int x[5];

  dynamic_span ds = x;
  static_assert(std::is_same_v<decltype(ds), p2998::span<int, p2998::dynamic_extent>>);
  f(x);

  const_dynamic_span cds = x;
  static_assert(std::is_same_v<decltype(cds), p2998::span<const int, p2998::dynamic_extent>>);
  g(x);

  const_span cs = x;
  static_assert(std::is_same_v<decltype(cs), p2998::span<const int, 5>>);
  h(x);
}
