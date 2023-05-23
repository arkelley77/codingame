#include "sim.h"

using namespace std;

using Timer = chrono::steady_clock;
using chrono::milliseconds;


void Pod::apply(int thrust, float angle_rad) {
  constexpr static float max_turn = radians(18.0);
  angle_rad = clamp(angle_rad, -max_turn, max_turn);
  dir.theta += angle_rad;
  dir.fixAngle();

  if (thrust == SHIELD) shield_frames = 4;
  else if (shield_frames > 0) return;
  else vel += (dir * thrust).xy();
}

void Pod::endFrame() {
  pos = { round(pos.x), round(pos.y) };
  vel = { trunc(vel.x * 0.85f), trunc(vel.y * 0.85f) };

  if (shield_frames > 0) --shield_frames;
}

float Pod::timeToCollision(const Pod& other) const {
  constexpr static float sr2 = 4 * pod_radius * pod_radius;

  if (vel == other.vel) return INFINITY;  // no relative velocity

  // solve the quadratic equation for collision time
  Vec2_xy d_pos = pos - other.pos;
  float dist2 = d_pos.r2();
  Vec2_xy d_v = vel - other.vel;
  float dv2 = d_v.r2();

  if (dv2 < dist2) return INFINITY;       // not moving fast enough
  
  float dot_prod = dot(d_pos, d_v);
  if (dot_prod < 0.0) return INFINITY;    // moving away from each other

  float neg_b = -2.0 * dot_prod;
  float disc = neg_b*neg_b - 4.0 * dv2 * (dist2 - sr2);

  if (disc < 0.0) return INFINITY;        // collision is imaginary

  float t = (neg_b - sqrt(disc)) / (2.0 * dv2);

  if (t <= 0.0) return INFINITY;          // collision happened last frame
  else return t;
}
float Pod::timeToCollision(const Checkpoint& cp) const {
  constexpr static float sr2 = checkpoint_radius * checkpoint_radius;

  if (vel.x == 0.0 && vel.y == 0.0) return INFINITY;

  // solve the quadratic equation for collision time
  Vec2_xy d_pos = pos - cp;
  float dist2 = d_pos.r2();
  float dv2 = vel.r2();

  if (dv2 < dist2) return INFINITY;       // not moving fast enough
  
  float dot_prod = dot(d_pos, vel);
  if (dot_prod < 0.0) return INFINITY;    // moving away from each other

  float neg_b = -2.0 * dot_prod;
  float disc = neg_b*neg_b - 4.0 * dv2 * (dist2 - sr2);

  if (disc < 0.0) return INFINITY;        // collision is imaginary

  float t = (neg_b - sqrt(disc)) / (2.0 * dv2);

  if (t <= 0.0) return INFINITY;          // collision happened last frame
  else return t;
}

void Pod::collide(Pod& other) {
    float m1 = mass(), m2 = other.mass();
    float mcoeff = (m1 * m2) / (m1 + m2);

    Vec2_xy d_pos = pos - other.pos;
    Vec2_xy d_v = vel - other.vel;

    Vec2_xy force = d_pos * (mcoeff * dot(d_pos, d_v) / d_pos.r2());

    Vec2_xy f1 = force * (1.0 / m1), f2 = force * (1.0 / m2);

    vel -= f1;
    other.vel += f2;

    float impulse = force.r();
    if (impulse < 120.0) {
      force *= 120.0 / impulse;
    }

    vel -= f1;
    other.vel += f2;
  }


// 0 if ongoing
// 1 if team 1 has won, 2 if team 2 has won
int SimState::outcome(const Field& map) const {
  for (int team = 0; team < 2; ++team) {
    for (int pod = 0; pod < 2; ++pod) {
      const Pod& p = pods[2 * team + pod];
      if (p.timeout <= 0) return 1 + (1 - team);
      else if (p.laps >= map.num_laps) return 1 + team;
    }
  }
  return 0;
}

void SimState::simFrame(const Field& map) {
  for (Pod& p : pods) --p.timeout;
  struct PodCollision {
    Pod *p1 = nullptr, *p2 = nullptr;
    float time = INFINITY;
  };
  float t = 0.0;
  do {
    // detect pod-on-pod collisions
    PodCollision pod_col;
    float col_time;
    for (auto it = pods.begin(); it != pods.end(); ++it) {
      for (auto it2 = it; it2 != pods.end(); ++it2) {
        col_time = it->timeToCollision(*it2);
        if (col_time < pod_col.time) {
          pod_col.p1 = it;
          pod_col.p2 = it2;
          pod_col.time = col_time;
        }
      }
    }
    // detect pod-on-checkpoint collisions
    for (Pod& p : pods) {
      col_time = p.timeToCollision(map.checkpoints[p.next_cp]);
      if (col_time < pod_col.time) {
        p.next_cp = (p.next_cp) % map.num_checkpoints;
        p.timeout = 100;
      }
    }

    if (pod_col.time < 1.0 - t) {
      for (Pod& p : pods) { p.move(pod_col.time); }
      pod_col.p1->collide(*pod_col.p2);
      t += pod_col.time;
    }
    else {
      col_time = 1.0 - t;
      for (Pod& p : pods) { p.move(col_time); }
      t = 1.0;
    }
  } while (t < 1.0);

  for (Pod& p : pods) { p.endFrame(); }
}

SimState SimState::next(const Field& map) const {
  SimState ret = *this;
  ret.simFrame(map);
  return ret;
}
