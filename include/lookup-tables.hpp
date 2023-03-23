#include <algorithm>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>


// lookup-tables.hpp
namespace kel {
# define rtype(fn) decltype(fn::eval(std::declval<size_t>()))
# define rtype2d(fn) decltype(fn::eval(std::declval<size_t>(), std::declval<size_t>()))

  template <class GenFn, size_t N, size_t... Indexes>
  static constexpr std::array<rtype(GenFn), N> _makeLookupTable(std::index_sequence<Indexes...>) noexcept {
    constexpr std::array<rtype(GenFn), N> table{ { GenFn::eval(Indexes)... } };
    return table;
  }
  template <class GenFn, size_t N, size_t... Indexes>
  static constexpr std::array<rtype(GenFn), N> makeLookupTable() noexcept {
    return _makeLookupTable<GenFn, N>(std::make_index_sequence<N>{});
  }

# define genLookupTable(gen_fn, size)                                                         \
struct gen_##gen_fn {                                                                         \
  static constexpr auto eval(size_t input) noexcept { return gen_fn(input); }                 \
};                                                                                            \
constexpr auto lut_##gen_fn = kel::makeLookupTable<gen_##gen_fn, size>()
# define lookup(gen_fn, index) (lut_##gen_fn[index])

# define genLookupTable2d(gen_fn, width, height)                                              \
struct gen_##gen_fn {                                                                         \
  static constexpr size_t w = width;                                                          \
  static constexpr auto eval(size_t input) noexcept { return gen_fn(input % w, input / w); }  \
};                                                                                            \
constexpr auto lut_##gen_fn = kel::makeLookupTable<gen_##gen_fn, width * height>()
# define lookup2d(gen_fn, x, y) (lut_##gen_fn[x + gen_##gen_fn::w * y])

# undef rtype
# undef rtype2d
}

// Example 1:
static constexpr int popcnt(size_t mask) noexcept {
  int count = 0;
  while (mask) {
    ++count;
    mask &= mask - 1; // reset the LS1B
  }
  return count;
}

genLookupTable(popcnt, 10);
constexpr int a = lookup(popcnt, 4);

// Example 2:
static constexpr int popcnt_of_2(size_t a, size_t b) noexcept {
  return lookup(popcnt, a) + lookup(popcnt, b);
}

genLookupTable2d(popcnt_of_2, 10, 10);
constexpr int b = lookup2d(popcnt_of_2, 7, 4);
