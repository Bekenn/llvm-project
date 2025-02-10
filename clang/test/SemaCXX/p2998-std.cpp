#include <span>

template <typename T> using const_span = std::span<const T>;
template <typename T> void f(std::span<const T>);

void example()
{
  int x[5];
  const_span cs = x;
  f(x);
}
