#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>

#include "../include/typedefs.hpp"
#include "../include/bit-fiddling.hpp"

using namespace std;
using namespace kel;

/***************************** User  Variables *******************************/

const char* array_type = "constexpr static u64";
const char* array_name = "z_x_board";
const char* array_size = "pow2(9)";
constexpr size_t num_elems = pow2(9);

using RNG = mt19937_64;                         // random number generator to use

constexpr bool console_output = false;          // whether to output to console
constexpr bool file_output = false;              // whether to output to file
const char* output_file = "C:\\Users\\arkel\\Desktop\\z_x_board.txt";
constexpr int row_len = 100;                    // maximum length of a row, in characters
constexpr int indentation = 6;                  // how many spaces at the beginning of the line are taken up by indentation

// define how each array element prints (& is converted) here
void print(RNG::result_type item, ostream& os) noexcept {
  os << "0x" << hex << setfill('0') << setw(16) << item;
}
constexpr int item_length = 2 + 16;             // maximum length of a single element, in characters

/*****************************************************************************/

constexpr int usable_char_cols = row_len - indentation - 2 - 1 + 1;   // 2 spaces, 1 comma, 1 unnecessary space
constexpr int _n_cols = usable_char_cols / (item_length + 1 + 1);     // 1 comma, 1 space
constexpr int n_cols = (_n_cols == 0) ? 1 : _n_cols;                  // print a minimum of 1 item per line

int main() {
  ofstream ofs;
  if constexpr (file_output) ofs.open(output_file);
# define write(thing) do {                                                    \
    if constexpr (console_output) cout << thing;                              \
    if constexpr (file_output) ofs << thing;                                  \
  } while (0)
# define indent() do {                                                        \
  for (int indent = 0; indent < indentation; ++indent) write(' ');            \
  } while (0)

  RNG rng;
  rng.seed(chrono::high_resolution_clock::now().time_since_epoch().count());
  indent();
  write(array_type << ' ' << array_name << '[' << array_size << "] = {\n");
  for (size_t idx = 0; idx < num_elems; ++idx) {
    indent();
    write("  ");
    for (size_t end_of_line = min(idx + n_cols, num_elems); idx < end_of_line; ++idx) {
      auto thing = rng();
      if constexpr (console_output) print(thing, cout);
      if constexpr (file_output) print(thing, ofs);
      if (idx != num_elems - 1) write(", ");
    }
    write('\n');
  }
  indent();
  write("};" << endl);
  if (ofs.is_open()) ofs.close();
}
