#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <random>
#include <set>
#include <stack>

#define USE_LOOKUP 1
#define USE_2D_LOOKUP 0

#define NUM_ROLLOUTS 2

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
#define clearLS1B(mask) (mask &= (mask - 1))
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


#if USE_LOOKUP
#define for_lookup static constexpr
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
#else
#define for_lookup inline
#define genLookupTable(gen_fn, size)
#define lookup(gen_fn, index) (gen_fn(index)))
#endif

#if USE_2D_LOOKUP
#define for_lookup2d static constexpr
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
#else
#define for_lookup2d inline
#define genLookupTable2d(gen_fn, width, height) void _____null()
#define lookup2d(gen_fn, x, y) (gen_fn(x, y))
#endif

#undef rtype
#undef rtype2d
}

using namespace kel;
using namespace std;

using chrono::high_resolution_clock, chrono::milliseconds, chrono::steady_clock;
using time_point = chrono::time_point<steady_clock>;

using bb = u16;

struct Coords {
  i32 x, y;
  bool operator==(const Coords& other) const {
    return reinterpret_cast<const i64&>(*this) == reinterpret_cast<const i64&>(other);
  }
};
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
#define localIdxToGlobalIdx_idx(local_idx, idx_of_local) (globalCoordsToIdx(localCoordsToGlobalCoords_idx(localIdxToCoords(local_idx), idx_of_local)))
#define globalIdxToLocalIdx_idx(idx) (localXyToIdx(globalIdxToX(idx) % 3, globalIdxToY(idx) % 3)), (localXyToIdx(globalIdxToX(idx) / 3, globalIdxToY(idx) / 3))

#define tof(__x) static_cast<float>(__x)

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
  ongoing = ' ',
  invalid = '*'
};

enum EasyWin : char {
  easy_x = 'X', easy_o = 'O', not_easy = ' '
};

for_lookup int popcnt(size_t mask) noexcept {
#if USE_LOOKUP
  int count = 0;
  while (mask)   {
    ++count;
    mask &= mask - 1; // reset the LS1B
  }
  return count;
#else
  return __builtin_popcntll(mask);
#endif
}
genLookupTable(popcnt, pow2(9));
for_lookup int bsf(size_t mask) noexcept {
  if (!mask) return -1;
#if USE_LOOKUP
  int count = 0;
  while ((mask & 1) == 0) { // check if LSB is set
    ++count;
    mask /= 2; // right shift
  }
  return count;
#else
  return __builtin_ctzll(mask);
#endif
}
genLookupTable(bsf, pow2(9));

for_lookup bool isWon(size_t board) noexcept {
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

for_lookup2d EasyWin easyWin(size_t x_b, size_t o_b) noexcept {
#define ALMOST_X(mask) (popcnt(x_b & mask & ~o_b) == 1)
#define ALMOST_O(mask) (popcnt(x_b & mask & ~o_b) == 1)
  return (
       ALMOST_X(diag_slash)
    || ALMOST_X(diag_back)
    || ALMOST_X(col_left)
    || ALMOST_X(col_middle)
    || ALMOST_X(col_right)
    || ALMOST_X(row_top)
    || ALMOST_X(row_middle)
    || ALMOST_X(row_bottom)
       ) ? easy_x : ((
       ALMOST_O(diag_slash)
    || ALMOST_O(diag_back)
    || ALMOST_O(col_left)
    || ALMOST_O(col_middle)
    || ALMOST_O(col_right)
    || ALMOST_O(row_top)
    || ALMOST_O(row_middle)
    || ALMOST_O(row_bottom)
       ) ? easy_o : not_easy);
#undef ALMOST_X
#undef ALMOST_O
}
genLookupTable2d(easyWin, pow2(9), pow2(9));
for_lookup2d WinState winState(size_t x_b, size_t o_b) noexcept {
  if (x_b & o_b) return invalid;
  else if (lookup(isWon, x_b)) return x_won;
  else if (lookup(isWon, o_b)) return o_won;
  else if (lookup(popcnt, (x_b | o_b) & ones(9)) == 9) return draw;
  else return ongoing;
}
genLookupTable2d(winState, pow2(9), pow2(9));
for_lookup2d bool isOngoing(size_t x_b, size_t o_b) noexcept {
  WinState w = lookup2d(winState, x_b, o_b);
  return w == ongoing && w != invalid;
}
genLookupTable2d(isOngoing, pow2(9), pow2(9));
for_lookup2d bool isTerminal(size_t x_b, size_t o_b) noexcept {
  WinState w = lookup2d(winState, x_b, o_b);
  return w != ongoing;
}
genLookupTable2d(isTerminal, pow2(9), pow2(9));

using MoveVector = vector<int>;

struct Board {
  bb x_board = 0, o_board = 0;
  bool operator==(const Board& other) const noexcept {
    return reinterpret_cast<const u32&>(*this)
      == reinterpret_cast<const u32&>(other);
  }
};

class UltimateBoard {
public:
  constexpr UltimateBoard() noexcept: locals(), next(-1), x_turn(true) {}

  Board getGlobal() const {
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
        break;
      default: break;
      }
    }
    return global;
  }

  void getMoves(MoveVector& moves) const {
    moves.clear();
    if (next != -1
      && !lookup2d(isTerminal, locals[next].x_board, locals[next].o_board)) {
      bb empty = ~(locals[next].x_board | locals[next].o_board);
      empty &= ones(9);
      do {
        moves.push_back(localIdxToGlobalIdx_idx(lookup(bsf, empty), next));
      } while (clearLS1B(empty));
    }
    else {
      Board global = getGlobal();
      bb open_locals = ~(global.x_board | global.o_board) & ones(9);
      if (open_locals) do {
        int local_idx = lookup(bsf, open_locals);
        bb empty = ~(locals[local_idx].x_board | locals[local_idx].o_board);
        empty &= ones(9);
        if (empty) do {
          moves.push_back(localIdxToGlobalIdx_idx(lookup(bsf, empty), local_idx));
        } while (clearLS1B(empty));
      } while (clearLS1B(open_locals));
    }
  }
  int getNumMoves() const {
    if (next != -1
      && !lookup2d(isTerminal, locals[next].x_board, locals[next].o_board)) {
      return lookup(popcnt, ~(locals[next].x_board | locals[next].o_board) & ones(9));
    }
    else {
      Board global = getGlobal();
      bb open_locals = ~(global.x_board | global.o_board) & ones(9);
      int num_moves = 0;
      if (open_locals) do {
        int local_idx = lookup(bsf, open_locals);
        bb empty = ~(locals[local_idx].x_board | locals[local_idx].o_board) & ones(9);
        num_moves += lookup(popcnt, empty);
      } while (clearLS1B(open_locals));
      return num_moves;
    }
  }
  UltimateBoard& mark(int idx, int idx_of_local) {
    if (x_turn) locals[idx_of_local].x_board |= localIdxToBB(idx);
    else locals[idx_of_local].o_board |= localIdxToBB(idx);
    next = (lookup2d(isTerminal, locals[idx].x_board, locals[idx].o_board))
      ? -1
      : idx;
    x_turn = !x_turn;
    return *this;
  }
  UltimateBoard copy() const { return *this; }

  bool operator==(const UltimateBoard& other) const noexcept {
    return (
      // don't strictly *need* to compare the turns,
      // but this short circuits 50% of the time
         x_turn == other.x_turn
      // this one short circuits 8/9 times
      && next == other.next
      // which means we only compare locals 1/18 times
      // so the probability of array comparison is < 6%
      && locals == other.locals
    );
  }

  struct hash {
    u64 operator()(const UltimateBoard& board) const noexcept {
      constexpr static u64 z_x_board[pow2(9)] = {
        0x276d46ec5066757c, 0x1ef454d54d15ba47, 0x2f2511163182666c, 0xe37e57d41d06f7fa,
        0x5de1b7290c214e6e, 0x115008f6bd7cd864, 0x27a65d7f3db8ebf9, 0x252e75a46548a419,
        0xa3a6f83b02597c05, 0x3c5a843ae8edcaf6, 0xa6601f21ce83d29a, 0x66d5c68d7d599608,
        0x21b67ad9e0032637, 0xa5a222b8cfe650fe, 0x27e08bb900a5f5dd, 0xf7eaff077787db15,
        0x1f307558cbcee970, 0xaf7703370debddb8, 0x8917f604609b2b42, 0x93a4e5a1c19b1ccd,
        0x52ae4cd64347cea6, 0x979d4f7c544ea323, 0xabdddc0928bf9305, 0x936c57e27a8264cd,
        0xa251d66785af44ac, 0x7243a6e1bc146ccf, 0x1534ec04cbb9a288, 0x536852f5c4eb33b2,
        0xb8dce39650ad6dc1, 0xf0e0a4dd7e2217ed, 0x074c3e57e364890c, 0xc8706ef600241439,
        0xf36bbf693def35ed, 0x60194f27643303a5, 0xd104177ff3649f4e, 0xdad52c6d2e487eaf,
        0xe2d6245085d235df, 0x7023d0f723117169, 0x26096eabf743983c, 0xefa56ad0257ae486,
        0xa02854bcfef6e0c3, 0xf79cf204b6ce83f9, 0x33554ac93634edea, 0x07ca57a187688dff,
        0xfcc9aa9a0086b00f, 0x5c7ceeb2e68637da, 0x335cca98d8b16af2, 0x933c1067566af73d,
        0x8061d266fddeed80, 0xffefb5678414486e, 0x384354b5b27e6d2e, 0xff5340c44f645bd0,
        0x5ef7952fdbaaa9da, 0xc913a5c169be50dd, 0x0ca693033d375990, 0x2d1d6cd15d469b86,
        0xc60a92b267760991, 0x53e53e43041c9b79, 0x8929c6a8ecc275a2, 0xd7f9282586fa8b66,
        0x81ab9eb39b05ecfb, 0x873288acdf294575, 0x9ae3d056676c931d, 0x38ce3a42b983231e,
        0x5ddff7237f2f0c61, 0x949a3ac905e8f9ab, 0xcd78a21158dede4a, 0x1e9b4cf67bddc51e,
        0xfc0041bdbeda140c, 0x32afc70348dcf1ba, 0x31f6722a12678a99, 0xa25a7f8fb64f9e0c,
        0x63c4119bbb40b9d6, 0x847c0ad0d75bcb1e, 0xbe893385ced51e70, 0x743d4b48ecd23beb,
        0xfb1f7c66d551e2bd, 0xcb431f777c637b70, 0x260205c114245d33, 0xaed7975dce0c5e71,
        0x3933e0a88a7a783b, 0xe3501334609ef553, 0xabf167ed49d4fab9, 0x5a147fa56bc169a3,
        0x092324d4f831285f, 0x75ede03ae7199cb4, 0x27323e6b36f81a44, 0xa16aa692f9709d47,
        0x2a7125618fda845b, 0x9f08dde81d79a5af, 0x5542474db7da3f95, 0x04afe8657f2e475f,
        0x552227ea15c94481, 0x53f5c792a1b4019d, 0xb2e1b3081b6db5cb, 0x88b39e2c0bead770,
        0xbd4cc6e4b2572b5b, 0x2209eba2ac275392, 0x21f9c18208695e01, 0x5a096b5031549880,
        0x6c561a169b04cd79, 0xaf2b0e674cab00e3, 0xf9d0de283b966ca6, 0x0473038d49e4ad6f,
        0x578df09a59458988, 0x2d78a837669d9f48, 0x4e4d05445417f0b0, 0xc12d89d273570009,
        0xda0eae94732abf1b, 0xa460701dc5459289, 0x8542467209717049, 0x228d9b5aebba16bd,
        0xd987eedd06d6784a, 0x56f33942e3b025b3, 0x3a247e484e27eed1, 0xf9404885096e4aa6,
        0xf046af36e9e8ee01, 0xeff4a2ead5532be8, 0x65e83e7f162203e4, 0x6d50041b3213b7c8,
        0x03125c271a22beb8, 0x9c91fc0ebde03926, 0x03d266353e002773, 0x2ed1df28db06e2dc,
        0xb4f831ac21a4e59b, 0x11f60960b87bc28b, 0x5069ed2fda880f4e, 0x9a28fbced7ace68e,
        0x055961b14e6ef4ef, 0xe8c92e765825032a, 0x6205043b9e647c7b, 0xdcfe9cdb987bbbeb,
        0xc56f33ad279a8037, 0x91860a56b992a91e, 0x25ef433d58c9c5ad, 0xc2f3bc9d061639d3,
        0x1c05b548dfa97cac, 0x424564e8c0a6799f, 0x2c5b00d53678838e, 0x48bfb86066f90c43,
        0xe37c671ae78c26ce, 0x958f57e94d0c9b5d, 0xfb9c4a1db32df4a0, 0x8fecd9db2f45cbce,
        0x8bafbcd54890b448, 0xadaac9cec8586169, 0x3b072d72a691d38f, 0xe5419ef8404b256e,
        0x27baa30b28287c91, 0x2bcf0a4048911e5b, 0x1a44d4358c62668b, 0x05d00f316f0d74bb,
        0x34b865c43da28e64, 0xab3199dcc9a5e071, 0x23a3467ac6c9d310, 0x3a3a5888236ba273,
        0x97b84ac427eb9093, 0xa4eaa214d9e02896, 0x708396c641f4f520, 0xc559b9dead28462f,
        0xaaaadee0979fe5c2, 0xe9465e544c10a91b, 0xe8eeee4227d539ed, 0xfb9ab8c044cb1adb,
        0x6ce81f2faa9138b5, 0x8f3bb22b2d982d27, 0x8be7f93c7e4ecb47, 0x2c0f22e334a7f440,
        0xa00ecd758f6273e8, 0x33f6d24823d72ada, 0xcfbe76a2090d7bb3, 0xe5822d57b1ff1c73,
        0x72868b5db374a8d1, 0xd341d4b3e9e83376, 0x7d24f52bd6d478d8, 0x38a8054e7ff3665f,
        0xa9738ac63157deb8, 0x245c40ae23f06c4a, 0x31c7cf248a05f32a, 0xddb004dee16d4411,
        0x3cb67ec68bf982f4, 0xbe68f4cc24870331, 0x14d18830edd1fedc, 0x3a4a66f815c43dbd,
        0xd8136251496d652f, 0x24028da1a59da23c, 0x527ebf827838d474, 0x35e7b40bb8257042,
        0x616b7c24ddd7b5ce, 0x81d12758880f25b5, 0x04f40a11bb819de2, 0xb01d5877772a687e,
        0x12568ad44b4772ad, 0xfab98b34104a0b38, 0x656387ca14ed39ff, 0x918cb23a984f99ab,
        0xc0b4153e6d303813, 0xad050fac36ac67ab, 0xc5f3e6b220c3fac4, 0x9b67c227e139b52b,
        0x7f4a98dd2cde579f, 0x20cd0c5018368b63, 0x39b19555b3561b5c, 0xcd4eab7e9e779869,
        0x3b7820da5224a8d3, 0xa7c1922a8c148bc1, 0xfd6a387ab583ba22, 0x36821160586a6816,
        0x5bc14375e3b9e00e, 0x591c2fefba1b6401, 0xc148c6986e09bfa0, 0x54ff401bf41a350f,
        0xbcd2bc2ccc26197c, 0x192ca66177ae5039, 0x3a1ed539379167da, 0x4e7d6aab9c493b6b,
        0x40fa9002abdacd47, 0xc930f87a5bf23cd0, 0xe0e7e2aec9b6f6df, 0x2c58fa1783ff9101,
        0x23cc5cbda6298654, 0x8a784c1c22b93509, 0xb285234ff0221219, 0xa02262ef42e1c7f2,
        0x8a01691f133ca004, 0x38b96ef33a92748f, 0xa128968ccb1746a8, 0x34094d0d708fa418,
        0x0b486c7544b9243c, 0x20168a63d1944425, 0x1ffb5d6464378d45, 0x9fbb3770275326f6,
        0x1e094f8988943bd4, 0xcf1261a897c37f1d, 0xe48eb3f6dae2f29e, 0x4b6dd9b6ee598f4f,
        0xb3c61e834f4a7d89, 0x8fcc6924ff035de6, 0x73e4ac2c24d902dd, 0xd1585d66e235fb3c,
        0xf548290680d11cd5, 0xc5573158ebc8a0ae, 0xdba3c9047c741034, 0x1f78c8b7b7b9689e,
        0xc8d0ce16c7acf561, 0x1f0b96826a0d6e9d, 0xe64011bac0837c2b, 0x0581a608c001b2cc,
        0xe4db8b11f4974a2a, 0xe9d69a9aa589e841, 0xbff2abb18d532ffc, 0x991b4fb1d60d2a5a,
        0xd5600cf53015a3ab, 0xb859dc10a568d4a7, 0xd761f06c69c5330b, 0x60b05b8607ebd2f8,
        0x35143ef6086402a8, 0x2b2852590f51423e, 0xa852a3b87ac3dcbe, 0x2e6547f363da5537,
        0x4b19170442061ced, 0xcf3366a73fed893c, 0x09264f9614c01fe3, 0xef00905a569a5a7a,
        0xeb09aa9e0fbf6dca, 0xa8f06ce57c6d0a81, 0xf6f1003ac4ae6713, 0xcbc162f49db7f942,
        0xc337a9f6a52fd453, 0x38ebb16df44e67c1, 0xaffba4c57cf91a70, 0xd896afe63a2137f9,
        0xbfb959de52302346, 0x489edb442609ba85, 0x8e421f84bea49fd3, 0x63e4d37ada76f68a,
        0x56cad147fa018946, 0x41ab92543cd04850, 0x61f34cd11c253f4e, 0x39f57f1718ec48b0,
        0xb460e479b305867b, 0xa6bb76fa775d29ec, 0x3367940580e83404, 0x481e297224272249,
        0xe7db53d32d6881ca, 0x71cb870bfa4eff72, 0xd011a33d807d4710, 0x7843e655275d84fe,
        0x42c6b8eb5a4fa9ff, 0x18dddad469c694bb, 0x5a6522ee84573584, 0x309c82048aed27c2,
        0xb143f87b7a3ba0a1, 0x6cf120292a409e84, 0x140675766541c151, 0x6787308e3ead01fe,
        0xcd19b2dc02064084, 0x5e52dfc056aab707, 0xac3670e397a297ee, 0x3abd81ca25a879a4,
        0x799ddeb26e5e7fe7, 0xdaf6e5596aacdf1d, 0x892f5320a3a6e57e, 0xddbf5b06733339bd,
        0x222730f0c763ee55, 0x57847e249a19b98a, 0x8d55f981478658ae, 0xcd04b10092515c77,
        0xc18dae2edcd24e75, 0x10908ae32c63d4f9, 0xdd787dba5c183231, 0x71019e748ca56f7d,
        0xce3ded0022e5cede, 0x951c53d97fbd49d6, 0xc93b61dd6375285a, 0x58e8b0c8ffb36438,
        0x686876583d9e17f1, 0xf389392e74368b42, 0x0b144f15b2d60c52, 0xa51b0a4654069c75,
        0x96241519dbd5d4c6, 0xfb64cc4adcb9453f, 0xdeb8d63512516ec8, 0xc5b439848087c050,
        0x24a363ee69da6055, 0xaad8bf36ae2a0e0d, 0xb702467a552db34c, 0x6a2832260f896bf0,
        0xc985d60f30056783, 0x5086351052c048e8, 0xf28cd2ee97fd9714, 0x4f3d82f6c483e698,
        0xf85cdd656e5d0c89, 0x4a48901a87d9a521, 0x4276f05a2324ac43, 0xb41cac6da161f0ed,
        0x0072ab766a4436d4, 0x66c5319024f07e64, 0x53eef0cd1b0df30f, 0x8ab90902f9610db1,
        0x180a6259efc385f2, 0x8500eb7b80144db8, 0x96ff630422c6418c, 0xea339a64cfff2581,
        0x5c8b3f7a3a26d219, 0xd9528bf53957777d, 0xee6eb3cb131f9711, 0x42150670fdca55c0,
        0x0b2d827647c6797e, 0x18ef86f8d2263dd9, 0xba07be8ba41a954d, 0xf3ba1a9d605eb4fb,
        0x43065be27fe076a1, 0xb0a1741a3cf4db85, 0x4db6bf25e54cbf25, 0x8ec6d197ea75a354,
        0x8c0ba5d137801630, 0x122ff77f8efc151a, 0x2f60eef5997479c4, 0x37d6912e3da190c2,
        0xff532631ab9e8b01, 0xd97aee154b7191fd, 0xa48d466b71feba35, 0x1604bc70e647e30a,
        0x82b0d2aaaaafcd6b, 0x4eba4ae13c3eff4f, 0x38e1d4a699735e3f, 0xba7ad5e8b167c986,
        0x99fbbdb167ac913c, 0xd004b21870b3e07f, 0x6e6e8913273d25c4, 0xdb4e6b24cdb14253,
        0xba9fc6f285da7801, 0xf68d36aee1c8c58d, 0x06d9f219da2b4077, 0xd536391821672cb3,
        0xab5d98ce08916eb8, 0xc45cde5e5543f0d6, 0xf05b11886406e991, 0x892da5647a789f96,
        0x5eab7e6967505d9a, 0x5e2d1d152b44497e, 0xc4385c8676a0fabd, 0x6ddc2a182e8294ff,
        0x93716d6f452e5b47, 0x74122474c28bd03a, 0xf4f948ff58a1cd85, 0x4745fbc6d878df5d,
        0x054d76072463648f, 0xc6ffc1f6f4d412a4, 0x0a4f01f6b22cc15f, 0xdb796da42334e452,
        0xb2f42bca3bbcc98e, 0x96297a3131276ddb, 0xc2ff98eb5e246f01, 0x8c825056da5c5c02,
        0x10234c6a82f640c3, 0x261256f751fee555, 0x50dce99754f963aa, 0xbd8035a7e7a06aca,
        0x6bb35e04abac64bf, 0x236134182e0584e8, 0xde858012f3b7695b, 0x98eb0beb8011ed35,
        0x741bbd2aac92913b, 0xe67009e2fb0940fb, 0xdd109997fa527859, 0x90bb2cb315d7db55,
        0x34d3f9b049f5f10c, 0xa5b2b7e450794c58
      };
      constexpr static u64 z_o_board[pow2(9)] = {
        0x99579b5333b18ffc, 0x51d438471efc7f96, 0xc0cd0708517e39be, 0x17e950922b9f67b8,
        0xe803726fc585c3b6, 0x7fdbc72cc09a9cd0, 0x0b037db7d8c40f59, 0x9d538e0575ef39de,
        0xf705d13c3c32d079, 0x88533707ba30465a, 0x568861c765fd7949, 0xcbc33d63cecd49f8,
        0x2fe3ce01fd01b3e0, 0x67ce9726fb460114, 0xb6d0274f28923ce4, 0xa5fc1bac83d88db2,
        0xac1abe9c083cba0f, 0xab8d4270af5560bb, 0x455faf02a2d612f4, 0x16b0a4dcdea357be,
        0x37e337c6b2430aba, 0xb602e99248c6c558, 0x387e976e8a0fddb2, 0x3a4114b75c3c39ae,
        0x106ad9d92e140169, 0x9c0d5dedeac988b8, 0x061853075d7c4473, 0x09ca0498156c36d4,
        0xc6179bbe1990d036, 0x791bb98c142d666f, 0x502a65886bed36ac, 0x643e155cbafc2347,
        0x60df1cff4288a7e0, 0xf04983fdfadf2c59, 0x28e2bc18fe8197b4, 0xca01537539b44ce6,
        0xca59cf8994fd9d64, 0xbf8a05a78906ad49, 0x343b74d7de643676, 0x72409c5144f2ca34,
        0xa320aaa6eb96b49d, 0xa2211b09f8d262c6, 0x48084e79d25ae672, 0xfa7a4000e34d1444,
        0xe6e0f3292b35e9da, 0xa7ca62a75c6d5e4d, 0xc451e73108ad5cd3, 0x7efd8873585429bb,
        0x1902100d31e253a5, 0x97ea4c4382c93f08, 0x023a6e44f5599cb1, 0x5adf4b12626fc5b8,
        0x8763074a48352d03, 0x9a1795d654d13102, 0xeb7a1cf22f172b46, 0xb80bfb068e5d32a2,
        0xfa375c71e520cce2, 0xf4f1830dbd771d7f, 0x1e216516fd4d439e, 0xfacd349e335a85b9,
        0x6bc6dcf94b8686f5, 0x6a4eac874fee2e46, 0x5cdec624c1f29acc, 0x70a262acda92c7d2,
        0x4746ef56cd4cb427, 0x8f7f5f05d024d5dd, 0x992e51cd2d85150c, 0x2118169c3307b9d9,
        0xe3e84ffba7103a93, 0x896333907747a295, 0x5f8f123fabf76336, 0x2b41bdfd1729a260,
        0xefe3e5fe9adf8266, 0x0d9d17d211f81bda, 0x0b85c59b3ce3c07e, 0xac36fe0059100fe6,
        0x23e6cc7759db4f73, 0xb471e3362976e43b, 0x1e9626c79627703f, 0x1ab2a2f7a3be7532,
        0xfff287ee6d9dbbd7, 0xb6e16a2f9e11468c, 0x542e5a4ba99b9569, 0xe03d9d2a26df2e85,
        0xe5e214808bf5c3dc, 0x0b8cef97e304322e, 0xd5589d554d1d584e, 0x51ddfbdb48d9728c,
        0xfeb6ad0c34ff4585, 0x437a589dd8d05920, 0x5f4665f48a767be8, 0x18b2a73aa8bbb5ad,
        0x7d42098e7fb34970, 0x9c41396c92e1e2ef, 0x903cd825b25684c9, 0x6a058657ec8c010c,
        0x21b452cb4a56333e, 0xcd91a199df8d812f, 0x6db6815d70eb38d6, 0xfa2baa8ec9bc0213,
        0x08bc1ed7ccc3ebd7, 0x12f122969b50e1d9, 0xb1a333ec64065f61, 0x86077c3eb930b474,
        0x5c5dc5951e834b1d, 0x1d06ceaa91c14f62, 0xf22240c6015ed0c4, 0x9bc652ca43c7fa58,
        0xb59e544368de0043, 0x8212241639032ec7, 0xff7478d8ca71947c, 0xe5b23653c5475991,
        0x0aa459369e18c467, 0xc2de3b6f6aa9c5c4, 0x62c1d73f273f6e29, 0x2f466c09cd13f283,
        0xcd8c9e896af2d80a, 0xabf4505461adb837, 0xa19c1c9db1a5cf2f, 0x0a53fc50027bbf92,
        0x933a0f9aeb98b908, 0xc10fafa1cb7fe5fc, 0x82254d4dc71aeb5b, 0x7f740374be1b6770,
        0xbc69adcfed13b3f5, 0xc231afcaea182186, 0xd6e5810e975fbb6a, 0x1945efb35060b1fb,
        0x55e0b014438621de, 0x84bd6615fc0e08a4, 0x1deeab7ce346f4dc, 0x213094b8ce4d026b,
        0x6052110edbfcde76, 0xb64e36b073d1fac3, 0xeae3673f2ea6e70d, 0xe416fddb6e5c1b08,
        0xad59622f8c1700b5, 0x007f24939e393f33, 0xdd0adff079ad8e03, 0xb4f05ac8934b17e3,
        0x6dba597b97bfaaf5, 0x8cc1ba11298d3ac0, 0x9d5e390890adfa5b, 0xf3f838b633011fe4,
        0xda5d3b3fca9166bf, 0xd51b9168f55cba8f, 0x615dd529bfb0c930, 0x90dfcad6fccf8c4b,
        0x90dd57f30bb029aa, 0x33e60a0b64f81737, 0x3f400df2e0c01fdc, 0xa00584553c682bd6,
        0xa20f539a957ab509, 0x3453d4c2a913aa18, 0xd0309bdce067bbc2, 0x13d1bd1cd199f63e,
        0x416a7deb794abdab, 0xd7bf1e8565f64470, 0xf8a50be54cb19334, 0x21383d290ee9a00c,
        0xea435175a67d0e57, 0xcd2266f39705cbc6, 0xcdf7f124999fd82a, 0xac786fafe6d68af3,
        0x9f0529b96564d5db, 0x3b38adc2af2785f6, 0xd27b215498a36a79, 0xdfffdef2b1152dfb,
        0xcc8c07796bad71ff, 0x1ac80844af99caa8, 0x158ea0a3acf7ea22, 0x7f3c8e80de55c37c,
        0x8ffc27ffa976b40a, 0xbfbb8748f57d2202, 0x4ea52eaa0421b095, 0x6a270f8a5e36ef0f,
        0xe3c2d82a227cd317, 0xe6b2593ec708b8d0, 0x8d7c8b1e1d64517e, 0x1a7e396c0ac448f5,
        0x1d27b944b898d189, 0xa292c145ddb60973, 0x6ef3cee7944f79f2, 0xecc668d9a1785bdf,
        0xa8b953c5bb8b143a, 0x1c2e814d9fa4a192, 0x58ad811fa03aa167, 0xa6df2bfb7a56c832,
        0x8236ca1fa9617b07, 0x42f19cfeea706f67, 0xf8a979404b95aaef, 0x3533a51c69249aa4,
        0xf575d8d8cb114bbf, 0x345cc4f8b6bd892b, 0x6aed37c42e729905, 0x6b912c8d8acf0b00,
        0xfc81f39eddff475a, 0xde36074c71f70ea1, 0x4ce0532cc79b574c, 0xfbd9d363cc11f850,
        0xd970dffaaf62fec5, 0x5e0b3d74a8e551bd, 0x04fcf52334d46c32, 0x4ef26a7d2b3bd0ee,
        0xd67ab30381fe8313, 0x8cbfb2d0d4be6caf, 0xb112953f4412ed84, 0x306663a927767ed9,
        0xd4fea1e6aa3f36e1, 0xd3d734268bd7bab7, 0x49e1c0cc36a24ac0, 0xbb901481d8df04d3,
        0xf69411df67150aab, 0x2e40f9e0f796b18d, 0x80779b9fb8a0a9b3, 0xc07aa1adc70b638f,
        0xf00364575e1d9e5e, 0x8bf7a1c2293e0c25, 0x2d4fd620ca0eed61, 0x6a03b14b3caa76f3,
        0xba1c6c5eb897ac78, 0xa2993ee96fd5fd07, 0xd494530ffd8f58a8, 0xce0165ffcb05a404,
        0x7b7267786a475b01, 0xf896fef7fcad6b11, 0x348f6de621939068, 0x2f9eca88eb5f839f,
        0xbfade35a8e1590a5, 0x3ab2192d3222e3e9, 0x5cd947a0b1c79c5f, 0x12fc6614f5370448,
        0x2632b0109bfe2eaa, 0x50a1b4b3c9bb8dde, 0xb342f452a1fac4ce, 0x76e18dfb2e953390,
        0x890eaba2000cc953, 0x784c3e978958545e, 0x6526c71667e62785, 0xb48b0d77b54717ac,
        0x2f62778d46ad0630, 0x0f2d98553922ee4d, 0xc26c495f75f673b8, 0x9156f99429a6571d,
        0x3d755c1321ac0f8c, 0x91489ebc7b11c30e, 0x88f438374c5a9abe, 0x66fcd978b4f99b35,
        0x367a6b3c6fb856db, 0x75aacd3a906a1da6, 0xed37a7a2ffa18473, 0x0d21e8cf1feed9ff,
        0x2b658b3dae6f6224, 0x2f078f22a8444481, 0xdc45a9909f677c56, 0xc393218e34ff02f8,
        0x363cbc124c7a6577, 0x0058dae277a6a563, 0xeb34a604c189d78c, 0xcf993889458c3e12,
        0x739ae0fb66968b49, 0xec2cdf81f390e8b1, 0x44730419646adbcb, 0x152ac5b3a52e5084,
        0x0e1eee3e799aa292, 0x90b3cfd04d023d2c, 0x5531e87d2a1cc494, 0x333b3e538ff8b312,
        0xec04aff0a21d2fd6, 0x8f1f2b3801ab32be, 0x8dc98b2c8758012d, 0x0355a97376840214,
        0x5c7a16e93eda6a1e, 0x6d5eb59528333614, 0x028627c4ced1659a, 0x1bc3f0a0eeeab9a5,
        0x76e8740e5de10031, 0x762039cd25e42da9, 0x383ace77245c8c05, 0xf86e86e1280fe373,
        0x3c074153973fb5a3, 0x6b11020bcf3f6022, 0x482b3625f50d2221, 0xd5033b4ac06f1858,
        0x310111c5f355c293, 0x2a5e6c2fe4dbfe51, 0x5736fa5c674af216, 0xfa3fa7c1bdc8e64e,
        0x391dba39391b0665, 0x5d727e6070a3b596, 0x954bc0cfc5676fb0, 0xb6245026174b2753,
        0x207545f6c7b16a5d, 0xc0b425ea0667ec5c, 0x8f6fc4aea68785d8, 0x15aceed4feb76065,
        0x6024c42efc473b2b, 0x45320b132a64be0a, 0x99b5afcd6695ea33, 0xaedde9ec5f31149c,
        0xe0a5d67e4b11cb4f, 0x6b716e0e10ffe267, 0x64c7cae32e6fe4de, 0xfb8baa9582eccc89,
        0xb5248a8e9311429a, 0x7593805e286e7d41, 0xcf95d6147fcadf05, 0x38042829e35e0826,
        0xd74f12888fd8b390, 0x8d0c169092de2d46, 0x92db5fce7c27c109, 0x70e7395062e8a495,
        0xa2d78e2fc0923a5e, 0xc18f94a297714f4f, 0xd8fffe903e0f6aa7, 0xa0b4aad0fb2dc45a,
        0x2156f9b7a1103f6b, 0x459e71fc2aa4a14f, 0x6602d55093b6ceb4, 0xa02f5a23035addad,
        0x350d3263fcc7797f, 0x77a3c9d58c731c99, 0xabae197a20cb4c9f, 0xd1b89ded2b22e36b,
        0xd204eb941326652c, 0x4c9cd3808a76bbb0, 0x48cc727d44875666, 0xfeb17dd7de1464ae,
        0x260905d663ac75bd, 0xefb1dbf51b954d0b, 0x9c7d4eb732978103, 0x7fe2d77215d8f3ba,
        0x791602f41a68e0aa, 0x1c720f18cf1dc8bb, 0x7170810a4c5318d6, 0x365e3e07443898e0,
        0x279c6cdc02096fb9, 0xc9d5f8f6aa9140f8, 0x8d383a5171abdd1e, 0x8821afff42c2367d,
        0xbb4a74794882c6e5, 0x913cd00400209e35, 0xd0be650bacd02c6b, 0x4fd6f41287ebd564,
        0xbcdd19037a1818a6, 0x97cb05bc742a0037, 0x181b749d3840c89d, 0x883f523ea76c9690,
        0x7fa22c67cc04e1d0, 0x608ce772bb42be00, 0xce1eb3c807cc2e8e, 0xf46bd989ffdc7393,
        0x2349209c201e59a1, 0xdcc66d57946e9c31, 0xbd1c5b26804ab88e, 0xa9331c33a48d0542,
        0xb15af038eb4cb86d, 0x66235aeff1813c08, 0x514fca0fe81dee8e, 0xd4b9b875d9006944,
        0x2b4e8a887bb1ff0d, 0x60e10a5340b38b5e, 0x7035620c8782e907, 0xceab4ea517abf682,
        0xd02a00cc377bbd3b, 0x44ea40fbff3d3354, 0xec5ecac361b3ad40, 0x25f754dca7fb34c1,
        0xc6cb4fa2eda7b068, 0x1416615e28493b74, 0xd9685f4145982b4a, 0x7cfb686dbf9c4dc2,
        0x953b2f3a7077bf94, 0xc96a52d8db33521e, 0xd4e37cf1b97744f2, 0x1382ad366a81d74d,
        0xfa187fce564cfb92, 0x0995b7998ab6c155, 0x291d1b6c5ff6ebd8, 0xb1c45fe274d4d02a,
        0xd20932da5d595613, 0xa89d98623fe9235d, 0xffb63150027fcbe3, 0x1dcf1e5a8cdcd789,
        0x3cac004bd2828d70, 0x0d77a32fbb198c30, 0x15d092d124fdcca9, 0xa3b29c5417f4fefb,
        0x2d61d89169cd95ee, 0x033736fc4c288911, 0xc9a0458ed42f5485, 0x677cbecd67af8bef,
        0xcd243756e95bb142, 0xbff722b314e543fe, 0xbcb7fd564dcc5405, 0x5794e5ff478f781a,
        0x35aa573c1453b939, 0x06ad3cdee2cab11d, 0x9520b3f0e9bd4754, 0x93e9daca0addd235,
        0x573fb2bc5171e9f7, 0x22f59462629747ab, 0xf8c28c2c1dde912e, 0xd1241191b96656af,
        0x6847ec27a569ce7e, 0x8634640a298418f1, 0xfad52f94a11b6f44, 0xd9738d8a18f69f85,
        0x4161ff9fbe9cf910, 0xd6d0f947404580a5
      };
      constexpr static u64 z_next[9] = {
        0x6ca4fb9924f5a045, 0xd176cc9ca768faa0, 0xb42084b6140f7f20, 0x957a2ea15485eaac,
        0xd57ece5c12f3ae29, 0xac649769a3ec6e58, 0x3d7cf30f45f15b15, 0x338bdc2ab1bcea44
      };
      constexpr static u64 z_turn[2] = {
        0x44629b8ae60b7ee7, 0x8bf20f6937e6ccd1
      };

      u64 result = 0;
      for (const Board& it : board.locals) {
        result ^= z_x_board[it.x_board];
        result ^= z_o_board[it.o_board];
      }
      result ^= z_next[board.next];
      result ^= z_turn[board.x_turn];
      return result;
    }
  };

//private:
  array<Board, 9> locals;
  i8 next;
  bool x_turn;
};


class MonteCarlo {
  struct Node {
    struct Child {
      Node* child;
      Node* parent;
      int move;
      int visits;
      Child(Node* child, Node* parent, int move) : child(child), parent(parent), move(move), visits(0) {}
      float ucb1() const {
        constexpr static float bias = 1.41421356237f;//sqrt(2);
        return (tof(child->wins) / child->sims) + bias * sqrt(tof(parent->sims) / visits);
      }
    };
    UltimateBoard board;
    int sims, wins;
    vector<Child> children;

    int parent_count;               // for maintenance of the transposition table
    Node(const UltimateBoard& board) : board(board), children(), parent_count(1) {}
  };
public:
  MonteCarlo(const UltimateBoard& state) : position_table(103), root(&emplace(state)), rng() {
    rng.seed(chrono::high_resolution_clock::now().time_since_epoch().count());
  }

  void updateState(const UltimateBoard& state) {
    auto it = position_table.find(state);
    if (it == position_table.end()) {
      // clear the tree & make a new root
      position_table.clear();
      root = &emplace(state);
    }
    else {
      // the position is in the tree; find it
      Node* new_root = nullptr;
      for (auto& child : root->children) {
        if (child.child->board == state) {
          new_root = child.child;
        }
        else erase(child.child);
      }
      if (new_root == nullptr) {
        position_table.clear();
        root = &emplace(state);
      }
      else {
        position_table.erase(root->board);
        root = new_root;
      }
    }
  }

  size_t runSearch(time_point timeout) {
    size_t loop_count = 0;
    MoveVector moves;       // this vector gets passed down the stack to avoid constructor/destructor thrashing
    moves.reserve(81);
    stack<Node*> visited;
    while (steady_clock::now() < timeout) {
      Node* node = root;

      // selection phase
      while (node->children.size() == node->board.getNumMoves() && node->children.size() != 0) {
        visited.push(node);
        node = selectNext(node);
      }

      // expansion phase
      Board b = node->board.getGlobal();
      if (lookup2d(winState, b.x_board, b.o_board) == ongoing && node->board.getNumMoves() > 0) {
        node = expand(node, visited, moves);
        moves.clear();
      }

      for (int i = 0; i < NUM_ROLLOUTS; i++) {
        // rollout phase
        WinState outcome = rollout(node, moves);

        // backprop phase
        backprop(node, outcome, visited);
      }

      ++loop_count;
    }
    return loop_count;
  }

  int getBest() {
    Node::Child* best = nullptr;
    int most_visits = -1;
    for (auto& child : root->children) {
      if (child.visits > most_visits) {
        best = &child;
        most_visits = child.visits;
      }
    }
    return best->move;
  }

private:
  unordered_map<UltimateBoard, Node, UltimateBoard::hash> position_table;
  Node* root;
  mt19937_64 rng;

  inline Node& emplace(const UltimateBoard& state) {
    auto [it, worked] = position_table.emplace(state, Node(state));
    return it->second;
  }

  void erase(Node* node) {
    // traverse the tree and remove nodes rooted at node
    for (auto& child : node->children) {
      if (child.child->parent_count == 1) erase(child.child);
      else --child.child->parent_count;
    }
    position_table.erase(node->board);
  }

  static Node* selectNext(Node* node) {
    auto it = node->children.begin();
    auto best = it;
    float best_score = best->ucb1();
    for (; it != node->children.end(); ++it) {
      float score = it->ucb1();
      if (score > best_score) {
        best = it;
        best_score = score;
      }
    }
    best->visits += NUM_ROLLOUTS;
    return best->child;
  }

  Node* expand(Node* node, stack<Node*>& visited, MoveVector& moves) {
    node->board.getMoves(moves);
    size_t move_idx = 1 + (rng() % (moves.size() - node->children.size()));
    int next_move;
    for (int& move : moves) {
      bool already_tried = false;
      for (auto& it : node->children) {
        if (it.move == move) already_tried = true;
      }
      if (!already_tried) {
        --move_idx;
        if (move_idx == 0) next_move = move;
      }
    }
    Node* next_node = &emplace(node->board.copy().mark(globalIdxToLocalIdx_idx(next_move)));
    node->children.push_back({ next_node, node, next_move });
    moves.clear();
    return next_node;
  }

  WinState rollout(const Node* node, MoveVector& moves) {
    UltimateBoard board = node->board;
    Board glob = board.getGlobal();
    while (!lookup2d(isTerminal, glob.x_board, glob.o_board) && board.getNumMoves() > 0) {
      if (board.next != -1) {
        EasyWin w = lookup2d(easyWin, board.locals[board.next].x_board, board.locals[board.next].o_board);
        if (board.x_turn && w == easy_x) return x_won;
        else if (!board.x_turn && w == easy_o) return o_won;
      }
      else {
        for (int i = 0; i < 9; ++i) {
          EasyWin w = lookup2d(easyWin, board.locals[i].x_board, board.locals[i].o_board);
          if (board.x_turn && w == easy_x) return x_won;
          else if (!board.x_turn && w == easy_o) return o_won;
        }
      }
      board.getMoves(moves);
      int next_move = moves[rng() % moves.size()];
      board.mark(globalIdxToLocalIdx_idx(next_move));
      glob = board.getGlobal();
      moves.clear();
    }
    WinState out = lookup2d(winState, glob.x_board, glob.o_board);
    if (out == ongoing) return draw;  // the global may be ongoing but there still aren't any moves left
    else return out;
  }

  static void backprop(Node* node, WinState outcome, stack<Node*>& visited) {
    bool winner_node = false;
    bool winning_outcome = outcome == x_won || outcome == o_won;
    do {
      ++node->sims;
      if (winner_node && winning_outcome) ++node->wins;
      winner_node = !winner_node;
      if (visited.empty()) break;
      node = visited.top();
      visited.pop();
    } while (true);
  }
};

void validateMovegen(UltimateBoard& board) {
  bool is_valid = true;
  vector<int> generated_moves;
  board.getMoves(generated_moves);
  int valid_action_count;
  cin >> valid_action_count; cin.ignore();
  if (valid_action_count != generated_moves.size()) {
    cerr << "Incorrect number of moves!" << endl;
    is_valid = false;
  }
  vector<int> given_moves;
  int row, col;
  for (int i = 0; i < valid_action_count; ++i) {
    cin >> row >> col; cin.ignore();
    given_moves.push_back(globalXyToIdx(col, row));
    if (find(generated_moves.begin(), generated_moves.end(), globalXyToIdx(col, row)) == generated_moves.end()) {
      cerr << "Failed to generate valid move: " << col << ' ' << row << endl;
      is_valid = false;
    }
  }
  for (int& it : generated_moves) {
    if (find(given_moves.begin(), given_moves.end(), it) == given_moves.end()) {
      cerr << "Generated invalid move: " << globalIdxToX(it) << ' ' << globalIdxToY(it) << endl;
      is_valid = false;
    }
  }
  if (!is_valid) {
    cerr << "generated:\n";
    for (auto& it : generated_moves) {
      cerr << "  " << globalIdxToX(it) << ' ' << globalIdxToY(it) << '\n';
    }
    cerr << "valid:\n";
    for (auto& it : given_moves) {
      cerr << "  " << globalIdxToX(it) << ' ' << globalIdxToY(it) << '\n';
    }
    throw std::runtime_error("Invalid move generation");
  }
}

int main() {
  UltimateBoard board;
  int opponent_row = -1, opponent_col = -1;
  cin >> opponent_row >> opponent_col; cin.ignore();
  if (opponent_row != -1 && opponent_col != -1) {
    board.mark(globalIdxToLocalIdx_idx(globalXyToIdx(opponent_col, opponent_row)));
  }
  validateMovegen(board);
  time_point start = steady_clock::now();
  MonteCarlo mcts(board);
  auto nsims = mcts.runSearch(start + milliseconds(950));
  while (true) {
    cerr << "Performed " << nsims << " expansions" << endl;
    int best = mcts.getBest();
    cout << globalIdxToY(best) << ' ' << globalIdxToX(best) << endl;
    board.mark(globalIdxToLocalIdx_idx(best));
    mcts.updateState(board);

    cin >> opponent_row >> opponent_col; cin.ignore();
    board.mark(globalIdxToLocalIdx_idx(globalXyToIdx(opponent_col, opponent_row)));
    validateMovegen(board);
    start = steady_clock::now();
    mcts.updateState(board);
    nsims = mcts.runSearch(start + milliseconds(75));
  }
  return 0;
}
