#include "nn.h"

using namespace std;

template <size_t in_size, size_t out_size>
array<float, out_size>& LinearLayer<in_size, out_size>::operator()(const array<float, in_size>& input) {
  auto node_weights = weights.begin();
  auto bias = biases.begin();
  auto node = output.begin();
  while (node != output.end()) {
    *node = *(bias++);
    auto in = input.begin();
    auto weight = (node_weights++)->begin();
    while (in != input.end()) {
      *node += *(weight++) * *(in++);
    }
    ++node;
  }

  return output;
}


array<Move, 2> Agent::race(const Field& map, const SimState& state, bool mine) {
  const Pod& my_p1 = (mine) ? state.pods[0] : state.pods[2];
  const Pod& my_p2 = (mine) ? state.pods[1] : state.pods[3];
  const Pod& their_p1 = (mine) ? state.pods[2] : state.pods[0];
  const Pod& their_p2 = (mine) ? state.pods[3] : state.pods[1];

  Vec2_pol p1_vel = my_p1.vel.polar(), p2_vel = my_p2.vel.polar();
  Vec2_pol p1_cp = (map.checkpoints[my_p1.next_cp] - my_p1.pos).polar();
  Vec2_pol p2_cp = (map.checkpoints[my_p2.next_cp] - my_p2.pos).polar();
  float p1_boost_av = my_p1.can_boost, p2_boost_av = my_p2.can_boost;
  float p1_shield_countdown = my_p1.shield_frames, p2_shield_countdown = my_p2.shield_frames;
  float p1_timeout = my_p1.timeout, p2_timeout = my_p2.timeout;
  Vec2_pol p1_p2 = (my_p2.pos - my_p1.pos).polar();
  Vec2_pol p2_p1 = (my_p1.pos - my_p2.pos).polar();
  Vec2_pol p1_e1 = (their_p1.pos - my_p1.pos).polar();
  Vec2_pol p1_e2 = (their_p2.pos - my_p1.pos).polar();
  Vec2_pol p2_e1 = (their_p1.pos - my_p2.pos).polar();
  Vec2_pol p2_e2 = (their_p2.pos - my_p2.pos).polar();
  Vec2_pol p1_e1_vel = their_p1.vel.polar(), p1_e2_vel = their_p2.vel.polar();
  Vec2_pol p2_e1_vel = their_p1.vel.polar(), p2_e2_vel = their_p2.vel.polar();

  p1_vel.theta -= my_p1.dir.theta; p1_vel.fixAngle();
  p2_vel.theta -= my_p2.dir.theta; p2_vel.fixAngle();
  p1_cp.theta -= my_p1.dir.theta; p1_cp.fixAngle();
  p2_cp.theta -= my_p2.dir.theta; p2_cp.fixAngle();
  p1_p2.theta -= my_p1.dir.theta; p1_p2.fixAngle();
  p2_p1.theta -= my_p2.dir.theta; p2_p1.fixAngle();
  p1_e1.theta -= my_p1.dir.theta; p1_e1.fixAngle();
  p1_e2.theta -= my_p1.dir.theta; p1_e2.fixAngle();
  p2_e1.theta -= my_p2.dir.theta; p2_e1.fixAngle();
  p2_e2.theta -= my_p2.dir.theta; p2_e2.fixAngle();
  p1_e1_vel.theta -= my_p1.dir.theta; p1_e1_vel.fixAngle();
  p1_e2_vel.theta -= my_p1.dir.theta; p1_e2_vel.fixAngle();
  p2_e1_vel.theta -= my_p2.dir.theta; p2_e1_vel.fixAngle();
  p2_e2_vel.theta -= my_p2.dir.theta; p2_e2_vel.fixAngle();
  array<float, 34> input = {
    p1_vel.r, p1_vel.theta, p1_cp.r, p1_cp.theta,
    p1_boost_av, p1_shield_countdown, p1_timeout,
    p1_p2.r, p1_p2.theta,
    p1_e1.r, p1_e1.theta, p1_e1_vel.r, p1_e1_vel.theta,
    p1_e2.r, p1_e2.theta, p1_e2_vel.r, p1_e2_vel.theta,
    p2_vel.r, p2_vel.theta, p2_cp.r, p2_cp.theta,
    p2_boost_av, p2_shield_countdown, p2_timeout,
    p2_p1.r, p2_p1.theta,
    p2_e1.r, p2_e1.theta, p2_e1_vel.r, p2_e1_vel.theta,
    p2_e2.r, p2_e2.theta, p2_e2_vel.r, p2_e2_vel.theta
  };
  brain(input);
  out_layer(brain);
  //out_layer(in_layer);
  float p1_angle = out_layer.output[0], p2_angle = out_layer.output[4];
  float p1_thrust = out_layer.output[1], p2_thrust = out_layer.output[5];
  bool p1_boost = out_layer.output[2], p2_boost = out_layer.output[6];
  bool p1_shield = out_layer.output[3], p2_shield = out_layer.output[7];
  p1_angle = radians(18.f) * tanh(p1_angle); p2_angle = radians(18.f) * tanh(p2_angle);
  p1_thrust = 50.f * tanh(p1_thrust) + 50.f; p2_thrust = 50.f * tanh(p2_thrust) + 50.f;

  if (!isfinite(p1_thrust)) p1_thrust = 0.f;
  if (!isfinite(p2_thrust)) p2_thrust = 0.f;
  if (!isfinite(p1_angle)) p1_angle = 0.f;
  if (!isfinite(p2_angle)) p2_angle = 0.f;

  int p1_thrust_out, p2_thrust_out;
  if (p1_shield_countdown) p1_thrust_out = 0;
  else if (p1_boost_av && p1_boost) p1_thrust_out = 650;
  else p1_thrust_out = roundf(p1_thrust);

  if (p2_shield) p2_thrust_out = SHIELD;
  else if (p2_shield_countdown) p2_thrust_out = 0;
  else if (p2_boost_av && p1_boost) p2_thrust_out = BOOST;
  else p2_thrust_out = roundf(p2_thrust);


  return { Move{p1_thrust_out, p1_angle}, Move{p2_thrust_out, p2_angle} };
}
