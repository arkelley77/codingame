#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <random>

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
#define pow2(power) (1ull << (power))
#define ones(num) (pow2(num) - 1ull)
#define bitRange(nbits, start, stop) (ones(nbits, stop) & (~ones(nbits, start)))

#define getLS1B(mask) ((mask) & -(mask))
#define clearLS1B(mask) (mask &= (mask - static_cast<decltype(mask)>(1)))
}

// lookup-tables.hpp
namespace kel {
#define rtype(fn) decltype(fn::eval(std::declval<size_t>()))
#define rtype2d(fn) decltype(fn::eval(std::declval<size_t>(), std::declval<size_t>()))

  template <class GenFn, size_t N, size_t... Indexes>
  static constexpr std::array<rtype(GenFn), N> 
    _makeLookupTable(std::index_sequence<Indexes...>) noexcept {
    constexpr std::array<rtype(GenFn), N> table{{GenFn::eval(Indexes)...}};
    return table;
  }
  template <class GenFn, size_t N, size_t... Indexes>
  static constexpr std::array<rtype(GenFn), N> makeLookupTable() noexcept {
    return _makeLookupTable<GenFn, N>(std::make_index_sequence<N>{});
  }

#define genLookupTable(gen_fn, size)                                           \
  struct gen_##gen_fn                                                          \
  {                                                                            \
    static constexpr auto eval(size_t input) noexcept                          \
    {                                                                          \
      return gen_fn(input);                                                    \
    }                                                                          \
  };                                                                           \
  constexpr auto lut_##gen_fn = kel::makeLookupTable<gen_##gen_fn, size>()
#define lookup(gen_fn, index) (lut_##gen_fn[index])

#define genLookupTable2d(gen_fn, width, height)                                \
  struct gen_##gen_fn                                                          \
  {                                                                            \
    static constexpr size_t w = width;                                         \
    static constexpr auto eval(size_t input) noexcept                          \
    {                                                                          \
      return gen_fn(input % w, input / w);                                     \
    }                                                                          \
  };                                                                           \
  constexpr auto lut_##gen_fn =                                                \
    kel::makeLookupTable<gen_##gen_fn, width * height>()
#define lookup2d(gen_fn, x, y) (lut_##gen_fn[x + gen_##gen_fn::w * y])

#undef rtype
#undef rtype2d
}

using namespace kel;
using namespace std;

using bb = u16;

struct Coords { int x, y; };
#define localXyToIdx(x, y) ((x) + 3 * (y))
#define localCoordsToIdx(coords) (localXyToIdx((coords).x, (coords).y))
#define localIdxToX(idx) ((idx) % 3)
#define localIdxToY(idx) ((idx) / 3)
#define localIdxToCoords(idx) (Coords{localIdxToX(idx), localIdxToY(idx)})
#define localIdxToBB(idx) (1u << (idx))
#define localXyToBB(x, y) (localIdxToBB(localXyToIdx(x, y)))
#define localCoordsToBB(coords) (localXyToBB((coords).x, (coords).y))

#define globalXyToIdx(x, y) ((x) + 9 * (y))
#define globalCoordsToIdx(coords) (globalXyToIdx((coords).x, (coords).y))
#define globalIdxToX(idx) ((idx) % 9)
#define globalIdxToY(idx) ((idx) / 9)
#define globalIdxToCoords(idx) (Coords{globalIdxToX(idx), globalIdxToY(idx)})

#define localXToGlobalX_xy(x, board_x) ((x) + 3 * (board_x))
#define localYToGlobalY_xy(y, board_y) ((y) + 3 * (board_y))
#define localXToGlobalX_coords(x, board_coords) (localXToGlobalX_xy(x, (board_coords).x))
#define localYToGlobalY_coords(y, board_coords) (localYToGlobalY_xy(y, (board_coords).y))
#define localXToGlobalX_idx(x, board_idx) (localXToGlobalX_xy(x, localIdxToX(board_idx)))
#define localYToGlobalY_idx(y, board_idx) (localYToGlobalY_xy(y, localIdxToY(board_idx)))
#define localCoordsToGlobalCoords_xy(coords, board_x, board_y) (Coords{localXToGlobalX_xy((coords).x, board_x), localYToGlobalY_xy((coords).y, board_y)})
#define localCoordsToGlobalCoords_coords(coords, board_coords) (localCoordsToGlobalCoords_xy(coords, (board_coords).x, (board_coords).y))
#define localCoordsToGlobalCoords_idx(coords, board_idx) (localCoordsToGlobalCoords_coords(coords, localIdxToCoords(board_idx)))

enum Square : bb {
  top_left = localXyToBB(0, 0),
  top_middle = localXyToBB(1, 0),
  top_right = localXyToBB(2, 0),
  middle_left = localXyToBB(0, 1),
  center = localXyToBB(1, 1),
  middle_right = localXyToBB(2, 1),
  bottom_left = localXyToBB(0, 2),
  bottom_middle = localXyToBB(1, 2),
  bottom_right = localXyToBB(2, 2),
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
  x_won = 'X',
  o_won = 'O',
  draw = '-',
  ongoing = ' '
};

static constexpr int popcnt(size_t mask) noexcept {
  int count = 0;
  while (mask)   {
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
    mask /= 2; // right shift
  }
  return count;
}
genLookupTable(bsf, pow2(9));

static constexpr bool isWon(size_t board) noexcept {
#define ALL_SET(mask) ((board & mask) == mask)
  return (
       ALL_SET(diag_slash)
    || ALL_SET(diag_back)
    || ALL_SET(col_left)
    || ALL_SET(col_middle)
    || ALL_SET(col_right)
    || ALL_SET(row_top)
    || ALL_SET(row_middle)
    || ALL_SET(row_bottom));
#undef ALL_SET
}
genLookupTable(isWon, pow2(9));

static constexpr WinState winState(size_t x_b, size_t o_b) noexcept {
  if (lookup(isWon, x_b & ~o_b)) return x_won;
  else if (lookup(isWon, o_b & ~x_b)) return o_won;
  else if (lookup(popcnt, x_b | o_b) == 9) return draw;
  else return ongoing;
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

struct Board { bb x_board = 0, o_board = 0; };

class UltimateBoard {
  array<Board, 9> locals;
  i8 next;
  bool x_turn;

public:
  constexpr UltimateBoard() noexcept : locals(), next(-1), x_turn(true) {}

  WinState getState() const {
    Board global;
    for (int idx = 0; idx < 9; ++idx) {
      switch (lookup2d(winState, locals[idx].x_board, locals[idx].o_board)) {
      case x_won:
        global.x_board |= localIdxToBB(idx);
        break;
      case o_won:
        global.o_board |= localIdxToBB(idx);
        break;
      case draw:
        global.x_board |= localIdxToBB(idx);
        global.o_board |= localIdxToBB(idx);
        break;
      default: break;
      }
    }
    return lookup2d(winState, global.x_board, global.o_board);
  }

  void getMoves(vector<int> &moves) const {
    moves.clear();
    if (next != -1
      && !lookup2d(isTerminal, locals[next].x_board, locals[next].o_board)) {
      bb empty = ~(locals[next].x_board | locals[next].o_board);
      do {
        moves.push_back(lookup(bsf, empty));
      } while (clearLS1B(empty));
    }
  }
  void mark(int idx, int idx_of_local) {
    if (x_turn) locals[idx_of_local].x_board |= localIdxToBB(idx);
    else locals[idx_of_local].o_board |= localIdxToBB(idx);
    next = (lookup2d(isTerminal, locals[idx].x_board, locals[idx].o_board))
      ? -1
      : idx;
    x_turn = !x_turn;
  }
  UltimateBoard copy() const { return *this; }

  struct hash {
    static u64 z_board[pow2(9)][pow2(9)];
    static u64 z_next[9];
    static u64 z_x_turn[2];
    static void initZTable() {
      mt19937_64 z_rand;
      for (size_t x = 0; x < pow2(9); ++x) {
        for (size_t y = 0; y < pow2(9); ++y) {
          z_board[x][y] = z_rand();
        }
      }
      for (size_t nxt = 0; nxt < 9; ++nxt) {
        z_next[nxt] = z_rand();
      }
      z_x_turn[0] = z_rand();
      z_x_turn[1] = z_rand();
    }
    u64 operator()(const UltimateBoard& board) const {
      u64 result = 0;
      for (const Board& it : board.locals) {
        result ^= z_board[it.x_board][it.o_board];
      }
      result ^= z_next[board.next];
      result ^= z_x_turn[board.x_turn];
      return result;
    }
  };
};

class MonteCarlo {
  struct Node {
    UltimateBoard board;
    int sims, wins;
    map<int, Node&> children;
    Node(const UltimateBoard& board) : board(board), sims(0), wins(0), children() {}
  };
public:
  MonteCarlo() : position_table(103) {}

private:
  unordered_map<UltimateBoard, Node, UltimateBoard::hash> position_table;
};

int main() {
  return 0;
}