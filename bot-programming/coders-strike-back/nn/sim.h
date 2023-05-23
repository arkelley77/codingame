#ifndef SIM_H
#define SIM_H

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

constexpr static inline float degrees(float radians) { return radians * (180.0 / M_PI);}
constexpr static inline float radians(float degrees) { return degrees * (M_PI / 180.0); }

struct Vec2_pol;
struct Vec2_xy {
  float x, y;
  constexpr Vec2_xy() : x(), y() {}
  constexpr Vec2_xy(float x, float y) : x(x), y(y) {}

  constexpr float r() const { return sqrt(x * x + y * y); }
  constexpr float r2() const { return x * x + y * y; }

  constexpr float theta() const { return atan2(y, x); }

  constexpr Vec2_pol polar() const;

  constexpr bool operator==(const Vec2_xy& other) const { return x == other.x && y == other.y; }
  constexpr bool operator!=(const Vec2_xy& other) const { return x != other.x || y != other.y; }

  constexpr Vec2_xy operator*(float d) const { return { x * d, y * d }; }
  constexpr Vec2_xy& operator*=(float d) { x *= d; y *= d; return *this; }

  constexpr Vec2_xy operator+(const Vec2_xy& other) const { return { x + other.x, y + other.y }; }
  constexpr Vec2_xy operator-(const Vec2_xy& other) const { return { x - other.x, y - other.y }; }
  Vec2_xy& operator+=(const Vec2_xy& other) { x += other.x; y += other.y; return *this; }
  Vec2_xy& operator-=(const Vec2_xy& other) { x -= other.x; y -= other.y; return *this; }

  constexpr Vec2_xy operator-() const { return { -x, -y }; }
};
struct Vec2_pol {
  float r, theta;
  constexpr Vec2_pol() : r(), theta() {}
  constexpr Vec2_pol(float r, float theta) : r(r), theta(theta) {}

  constexpr float x() const { return r * cos(theta); }
  constexpr float y() const { return r * sin(theta); }

  constexpr Vec2_xy xy() const { return { x(), y() }; }

  constexpr void fixAngle() { theta = fmod(theta, M_2_PI); }

  constexpr float diffAngle(const Vec2_pol& other) const {
    float da = theta - other.theta;
    if (da > M_PI) da -= M_PI;
    if (da < -M_PI) da += M_PI;
    return da;
  }

  constexpr Vec2_pol operator*(float d) const { return { r * d, theta }; }
  Vec2_pol& operator*=(float d) { r *= d; return *this; }

  constexpr Vec2_pol operator-() const { return { -r, theta }; }
};
constexpr Vec2_pol Vec2_xy::polar() const { return { r(), theta() }; }

constexpr static inline float dot(Vec2_xy a, Vec2_xy b) { return a.x * b.x + a.y * b.y; }
constexpr static inline float dot(Vec2_pol a, Vec2_pol b) { return a.r * b.r * cos(a.theta - b.theta); }

enum SpecialThrust : int { SHIELD = -1, BOOST = 650 };
struct Move {
  int thrust;
  float angle_rad;
};

typedef Vec2_xy Checkpoint;
constexpr float checkpoint_radius = 600.0;

constexpr float pod_radius = 400.0;
struct Pod {
  Vec2_xy pos;        // position of the pod
  Vec2_xy vel;        // velocity vector
  Vec2_pol dir;       // direction vector (rotation angle)
  size_t next_cp;     // next checkpoint

  int laps;           // number of completed laps
  bool can_boost;     // whether the pod can still activate its boost
  int shield_frames;  // how many frames before the pod can accelerate or shield again
  int timeout;
  Pod() : pos(0.0, 0.0), vel(0.0, 0.0), dir(1.0, 0.0), next_cp(0),
    laps(0), can_boost(true), shield_frames(0), timeout(5000) {}

  float mass() const { return (shield_frames == 4) ? 10.0 : 1.0; }

  void apply(int thrust, float angle_rad);
  void apply(Move move) { apply(move.thrust, move.angle_rad); }

  void move(float t) { pos += vel * t; }

  void endFrame();

  float timeToCollision(const Pod& other) const;
  float timeToCollision(const Checkpoint& cp) const;

  void collide(Pod& other);
};

struct Field {
  int num_laps, num_checkpoints;
  std::vector<Checkpoint> checkpoints;

  friend std::istream& operator>>(std::istream& is, Field& f) {
    is >> f.num_laps;
    is >> f.num_checkpoints;
    f.checkpoints.resize(f.num_checkpoints);
    for (int i = 0; i < f.num_checkpoints; ++i) {
      is >> f.checkpoints[i].x >> f.checkpoints[i].y;
    }
    return is;
  }
};

struct SimState {
  std::array<Pod, 4> pods;
  SimState() : pods() {}

  friend std::istream& operator>>(std::istream& is, SimState& state) {
    for (Pod& p : state.pods) {
      is >> p.pos.x >> p.pos.y >> p.vel.x >> p.vel.y >> p.dir.theta >> p.next_cp;
      p.dir.theta = radians(p.dir.theta);
      p.dir.fixAngle();
    }
    return is;
  }

  // 0 if ongoing
  // 1 if team 1 has won, 2 if team 2 has won
  int outcome(const Field& map) const;

  void simFrame(const Field& map);

  SimState next(const Field& map) const;
};

#endif