#include <span>
#include <array>

template <typename T> using bad_array = std::array<T, std::dynamic_extent>;

template <typename T> void f(const std::array<T, std::dynamic_extent>&);
template <typename... Ts> void g(Ts... xs) { f(std::array{ xs... }); }
void example()
{
    g(1);
}
