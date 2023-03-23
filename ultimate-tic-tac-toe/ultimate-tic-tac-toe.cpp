#include <algorithm>
#include <cstdint>
#include <type_traits>

#define abs(x) (((x) < 0) ? -(x) : (x))

// typedefs.hpp
namespace kel {
  typedef uint8_t u8;
  typedef int8_t i8;
  typedef uint16_t u16;
  typedef int16_t i16;
  typedef uint32_t u32;
  typedef int32_t i32;
  typedef uint64_t u64;
  typedef int64_t i64;

  typedef unsigned int uint;
  typedef unsigned long ulong;
  typedef unsigned long long ull;
}

// bit-fiddling.hpp
namespace kel {
# define pow2(nbits, power) (static_cast<u##nbits##>(1) << (power))
# define ones(nbits, num) (pow2(nbits, num) - static_cast<u##nbits##>(1))
# define bitRange(nbits, start, stop) (ones(nbits, stop) & (~ones(nbits, start)))

# define getLS1B(mask) ((mask) & -(mask))
# define clearLS1B(mask) (mask &= (mask - static_cast<decltype(mask)>(1)))
}

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


using namespace kel, std;
using bb = u16;


struct Coords {
  int x, y;
  constexpr inline Coords() : x(), y() {}
  constexpr inline Coords(int x, int y) : x(x), y(y) {}

  bool operator==(const Coords& other) const { return x == other.x && y == other.y; }
};

struct IdxOnLocal : Coords {
  constexpr inline IdxOnLocal() : Coords() {}
  constexpr inline IdxOnLocal(int x, int y) : Coords(x, y) {}
  constexpr inline IdxOnLocal(int idx) : Coords(idx % 3, idx / 3) {}
  constexpr inline int toIdx() const { return x + 3 * y; }
  constexpr inline bb toBoard() const { return 1u << toIdx(); }
};
typedef IdxOnLocal LocalIdx;

struct IdxOnGlobal {
  IdxOnLocal coords;
  LocalIdx board;
  constexpr inline IdxOnGlobal() : coords(), board() {}
  constexpr inline IdxOnGlobal(const IdxOnLocal& coords, const LocalIdx& board) : coords(coords), board(board) {}
  constexpr inline IdxOnGlobal(int x, int y) : coords(x % 3, y % 3), board(x / 3, y / 3) {}
  constexpr inline int x() const { return coords.x + 3 * board.x; }
  constexpr inline int y() const { return coords.y + 3 * board.y; }
  constexpr inline Coords xy() const { return { x(), y() }; }

  bool operator==(const IdxOnGlobal& other) const { return coords == other.coords && board == other.board; }
};


enum Square : bb {
  top_left = IdxOnLocal(0, 0).toBoard(), top_middle = IdxOnLocal(1, 0).toBoard(), top_right = IdxOnLocal(2, 0).toBoard(),
  middle_left = IdxOnLocal(0, 1).toBoard(), center = IdxOnLocal(1, 1).toBoard(), middle_right = IdxOnLocal(2, 1).toBoard(),
  bottom_left = IdxOnLocal(0, 2).toBoard(), bottom_middle = IdxOnLocal(1, 2).toBoard(), bottom_right = IdxOnLocal(2, 2).toBoard(),
};
enum Row : bb {
  diag_slash = top_left | center | bottom_right, // /
  diag_back = top_right | center | bottom_left,  // \

  col_left = top_left | middle_left | bottom_left,
  col_middle = top_middle | center | bottom_middle,
  col_right = top_right | middle_right | bottom_right,

  row_top = top_left | top_middle | top_right,
  row_middle = middle_left | center | middle_right,
  row_bottom = bottom_left | bottom_middle | bottom_right,
};


enum WinState : char {
  x_won = 'X', o_won = 'O', draw = '-', ongoing = ' ',
  invalid_board = '*'
};


static constexpr int popcnt(size_t mask) noexcept {
  int count = 0;
  while (mask) {
    ++count;
    mask &= mask - 1; // reset the LS1B
  }
  return count;
}
genLookupTable(popcnt, pow2(9));
static constexpr int bsf(size_t mask) noexcept {
  if (!mask) return -1;
  int count = 0;
  while ((mask & 1) == 0) { // check if LSB is set
    ++count;
    mask /= 2;              // right shift
  }
  return count;
}
genLookupTable(bsf, pow2(9));

static constexpr IdxOnLocal findMark(size_t board) noexcept { return { lookup(bsf, board) }; }
genLookupTable(findMark, pow2(9));

static constexpr bool isWon(size_t board) noexcept {
# define ALL_SET(mask) ((board & mask) == mask)
  return (
    ALL_SET(diag_slash)
    || ALL_SET(diag_back)
    || ALL_SET(col_left)
    || ALL_SET(col_middle)
    || ALL_SET(col_right)
    || ALL_SET(row_top)
    || ALL_SET(row_middle)
    || ALL_SET(row_bottom)
    );
# undef ALL_SET
}
genLookupTable(isWon, pow2(9));

static constexpr WinState winState(size_t x_b, size_t o_b) noexcept {
  if (x_b & o_b || abs(popcnt::eval(x_b) - popcnt::eval(o_b))) return invalid_board;
  else {
    if (isWon::eval(x_b)) return x_won;
    else if (isWon::eval(o_b)) return o_won;
    else if (popcnt::eval(x_b | o_b) == 9) return draw;
    else return ongoing;
  }
}
genLookupTable2d(winState, pow2(9), pow2(9));
static constexpr bool isOngoing(size_t x_b, size_t o_b) noexcept {
  return lookup2d(winState, x_b, o_b) == ongoing;
}
genLookupTable2d(isOngoing, pow2(9), pow2(9));
static constexpr bool isTerminal(size_t x_b, size_t o_b) noexcept {
  return !lookup2d(winState, x_b, o_b);
}
genLookupTable2d(isTerminal, pow2(9), pow2(9));
