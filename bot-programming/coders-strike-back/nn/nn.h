#ifndef NN_H
#define NN_H

#include <algorithm>
#include <array>
#include <cmath>

#include "sim.h"

template <size_t in_size, size_t out_size>
struct LinearLayer {
  std::array<std::array<float, in_size>, out_size> weights;
  std::array<float, out_size> biases;
  std::array<float, out_size> output;

  std::array<float, out_size>& operator()(const std::array<float, in_size>& input);

  template <size_t _sz>
  std::array<float, out_size>& operator()(LinearLayer<_sz, in_size>& input) {
    for (float& f : input.output) f = tanh(f);
    return operator()(input.output);
  }
};

struct Agent {
  // inputs: (all vectors are polar, relative to heading) (per pod)
  // - velocity vector
  // - vector to next checkpoint
  // - boost allowed (1 or 0)
  // - on shield countdown (1 or 0)
  // - timeout remaining (100 - 0)
  // - vector to teammate
  // - teammate's velocity vector
  // - vector to enemy (x2)
  // - enemy's velocity vector (x2)
  // outputs: (for each pod)
  // - turn angle (-18.0 to 18.0)
  // - thrust amount (0.0 to 100.0)
  // - boost (0 or 1)
  // - shield (0 or 1)

  LinearLayer<34, 16> brain;
  LinearLayer<16, 8> out_layer;

  std::array<Move, 2> race(const Field& map, const SimState& state, bool mine);
};

#endif