#ifndef DEBUG
#  pragma GCC optimize("Ofast")
#  pragma GCC optimize("inline")
#  pragma GCC optimize("omit-frame-pointer")
#  pragma GCC optimize("unroll-loops")
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

using Timer = chrono::steady_clock;
using chrono::milliseconds;

/*
Rules from Coders Strike Back Gold:
  The players each control a team of two pods during a race.
  As soon as a pod completes the race, that pod's team is declared the winner.
  The circuit of the race is made up of checkpoints.
  To complete one lap, your vehicle (pod) must pass through each one in order and back through the start.
  The first player to reach the start on the final lap wins.

  The game is played on a map 16000 units wide and 9000 units high. The coordinate X=0, Y=0 is the top left pixel.

  The checkpoints work as follows:
    The checkpoints are circular, with a radius of 600 units.
    Checkpoints are numbered from 0 to N where 0 is the start and N-1 is the last checkpoint.
    The disposition of the checkpoints is selected randomly for each race.

  The pods work as follows:
    To pass a checkpoint, the center of a pod must be inside the radius of the checkpoint.
    To move a pod, you must print a target destination point followed by a thrust value.
      Details of the protocol can be found further down.
    The thrust value of a pod is its acceleration and must be between 0 and 100.
    The pod will pivot to face the destination point by a maximum of 18 degrees per turn
      and will then accelerate in that direction.
    You can use 1 acceleration boost in the race, you only need to replace the thrust value by the BOOST keyword.
    You may activate a pod's shields with the SHIELD command instead of accelerating.
      This will give the pod much more weight if it collides with another.
      However, the pod will not be able to accelerate for the next 3 turns.
    The pods have a circular force-field around their center, with a radius of 400 units,
      which activates in case of collisions with other pods.
    The pods may move normally outside the game area.
    If none of your pods make it to their next checkpoint in under 100 turns,
      you are eliminated and lose the game. Only one pod need to complete the race.



Advanced Rules from Coders Strike Back Gold:
  On each turn the pods movements are computed this way:
    Rotation: the pod rotates to face the target point, with a maximum of 18 degrees (except for the 1rst round).
    Acceleration: the pod's facing vector is multiplied by the given thrust value. 
      The result is added to the current speed vector.
    Movement: The speed vector is added to the position of the pod. 
      If a collision would occur at this point, the pods rebound off each other.
    Friction: the current speed vector of each pod is multiplied by 0.85
    The speed's values are truncated and the position's values are rounded to the nearest integer.

  Collisions are elastic. The minimum impulse of a collision is 120.
  A boost is in fact an acceleration of 650. The number of boost available is common between pods. 
    If no boost is available, the maximum thrust is used.
  A shield multiplies the Pod mass by 10.
  The provided angle is absolute. 0° means facing EAST while 90° means facing SOUTH.


I/O Format Info from Coders Strike Back Gold:
  Game Input
    Initialization input
      Line 1: laps : the number of laps to complete the race.
      Line 2: checkpointCount : the number of checkpoints in the circuit.
      Next checkpointCount lines : 2 integers checkpointX , checkpointY for the coordinates of checkpoint.
    Input for one game turn
      First 2 lines: Your two pods.
      Next 2 lines: The opponent's pods.
      Each pod is represented by 6 integers:
        x & y for the position. 
        vx & vy for the speed vector. 
        angle for the rotation angle in degrees. 
        nextCheckPointId for the number of the next checkpoint the pod must go through.
    Output for one game turn
      Two lines: 2 integers for the target coordinates of your pod followed by thrust , 
      the acceleration to give your pod, or by SHIELD to activate the shields, or by BOOST for an acceleration burst. 
      One line per pod.
  Constraints
    0 ≤ thrust ≤ 100
    2 ≤ checkpointCount ≤ 8
    Response time first turn ≤ 1000ms
    Response time per turn ≤ 75ms
*/


struct Vec2_xy;
struct Vec2_pol;
using Cartesian = Vec2_xy;
using Polar = Vec2_pol;

constexpr double degrees(double radians) { return radians * (180.0 / M_PI); }
constexpr double radians(double degrees) { return degrees * (M_PI / 180.0); }

double dot(Vec2_xy a, Vec2_xy b) { return a.x * b.x + a.y * b.y; }
double dot(Vec2_pol a, Vec2_pol b) { return a.r * b.r * cos(a.theta - b.theta); }

struct Vec2_xy {
  double x, y;
  Vec2_xy(double x, double y) : x(x), y(y) {}
  explicit operator Vec2_pol() const { return polar(); }

  double r() const { return sqrt(x * x + y * y); }
  double r2() const { return x * x + y * y; }

  double theta() const { return atan2(y, x); }

  Vec2_pol polar() const { return { r(), theta() }; }

  bool operator==(const Vec2_xy& other) const { return x == other.x && y == other.y; }
  bool operator!=(const Vec2_xy& other) const { return x != other.x || y != other.y; }

  Vec2_xy operator*(double d) const { return { x * d, y * d }; }
  Vec2_xy& operator*=(double d) { x *= d; y *= d; return *this; }

  Vec2_xy operator+(const Vec2_xy& other) const { return { x + other.x, y + other.y }; }
  Vec2_xy operator-(const Vec2_xy& other) const { return { x - other.x, y - other.y }; }
  Vec2_xy& operator+=(const Vec2_xy& other) { x += other.x; y += other.y; return *this; }
  Vec2_xy& operator-=(const Vec2_xy& other) { x -= other.x; y -= other.y; return *this; }

  Vec2_xy operator-() const { return { -x, -y }; }
};
struct Vec2_pol {
  double r, theta;
  Vec2_pol(double r, double theta) : r(r), theta(theta) {}
  operator Vec2_xy() const { return { x(), y() }; }

  double x() const { return r * cos(theta); }
  double y() const { return r * sin(theta); }

  Vec2_xy xy() const { return { x(), y() }; }

  void fixAngle() { theta = fmod(theta, M_2_PI); }

  double diffAngle(const Vec2_pol& other) const {
    double da = theta - other.theta;
    if (da > M_PI) da -= M_PI;
    if (da < -M_PI) da += M_PI;
    return da;
  }

  Vec2_pol operator*(double d) const { return { r * d, theta }; }
  Vec2_pol& operator*=(double d) { r *= d; return *this; }

  Vec2_pol operator-() const { return { -r, theta }; }
};


/* The checkpoints work as follows:
 *  The checkpoints are circular, with a radius of 600 units.
 *  Checkpoints are numbered from 0 to N where 0 is the start and N-1 is the last checkpoint.
 *  The disposition of the checkpoints is selected randomly for each race.
*/
struct Checkpoint : Vec2_xy {
  constexpr static double radius = 600.0;
};

/* The pods work as follows:
    To pass a checkpoint, the center of a pod must be inside the radius of the checkpoint.
    To move a pod, you must print a target destination point followed by a thrust value.
      Details of the protocol can be found further down.
    The thrust value of a pod is its acceleration and must be between 0 and 100.
    The pod will pivot to face the destination point by a maximum of 18 degrees per turn
      and will then accelerate in that direction.
    You can use 1 acceleration boost in the race, you only need to replace the thrust value by the BOOST keyword.
    You may activate a pod's shields with the SHIELD command instead of accelerating.
      This will give the pod much more weight if it collides with another.
      However, the pod will not be able to accelerate for the next 3 turns.
    The pods have a circular force-field around their center, with a radius of 400 units,
      which activates in case of collisions with other pods.
    The pods may move normally outside the game area.
    If none of your pods make it to their next checkpoint in under 100 turns,
      you are eliminated and lose the game. Only one pod need to complete the race.
  Each pod is represented by 6 integers:
    * x & y for the position. 
    * vx & vy for the speed vector. 
    * angle for the rotation angle in degrees. 
    * nextCheckPointId for the number of the next checkpoint the pod must go through.
*/
struct Pod {
  constexpr static double radius = 400.0;
  Vec2_xy pos;        // position of the pod
  Vec2_xy vel;        // velocity vector
  Vec2_pol dir;       // direction vector (rotation angle)
  size_t next_cp;     // next checkpoint

  int laps;           // number of completed laps
  bool can_boost;     // whether the pod can still activate its boost
  int shield_frames;  // how many frames before the pod can accelerate or shield again
  int timeout;
  Pod() : pos(0.0, 0.0), vel(0.0, 0.0), dir(1.0, 0.0), next_cp(0),
    laps(0), can_boost(true), shield_frames(0), timeout(100) {}

  double mass() const { return (shield_frames == 4) ? 10.0 : 1.0; }

  enum SpecialThrust : int { SHIELD = -1, BOOST = 650 };
  void apply(int thrust, double angle_rad) {
    constexpr static double max_turn = radians(18.0);
    angle_rad = clamp(angle_rad, -max_turn, max_turn);
    dir.theta += angle_rad;
    dir.fixAngle();

    if (thrust == SHIELD) shield_frames = 4;
    else if (shield_frames > 0) return;
    else vel += dir * thrust;
  }

  void move(double t) { pos += vel * t; }

  void endFrame() {
    pos = { round(pos.x), round(pos.y) };
    vel = { trunc(vel.x * 0.85), trunc(vel.y * 0.85) };

    if (shield_frames > 0) --shield_frames;
  }
  
  double timeToCollision(const Pod& other) const {
    constexpr static inline double sr2 = radius * radius;

    if (vel == other.vel) return INFINITY;  // no relative velocity

    // solve the quadratic equation for collision time
    Vec2_xy d_pos = pos - other.pos;
    double dist2 = d_pos.r2();
    Vec2_xy d_v = vel - other.vel;
    double dv2 = d_v.r2();

    if (dv2 < dist2) return INFINITY;       // not moving fast enough
    
    double dot_prod = dot(d_pos, d_v);
    if (dot_prod < 0.0) return INFINITY;    // moving away from each other

    double neg_b = -2.0 * dot_prod;
    double disc = neg_b*neg_b - 4.0 * dv2 * (dist2 - sr2);

    if (disc < 0.0) return INFINITY;        // collision is imaginary

    double t = (neg_b - sqrt(disc)) / (2.0 * dv2);

    if (t <= 0.0) return INFINITY;          // collision happened last frame
    else return t;
  }
  double timeToCollision(const Checkpoint& cp) const {
    constexpr inline static double sr2 = cp.radius * cp.radius;

    if (vel.x == 0.0 && vel.y == 0.0) return INFINITY;

    // solve the quadratic equation for collision time
    Vec2_xy d_pos = pos - cp;
    double dist2 = d_pos.r2();
    double dv2 = vel.r2();

    if (dv2 < dist2) return INFINITY;       // not moving fast enough
    
    double dot_prod = dot(d_pos, vel);
    if (dot_prod < 0.0) return INFINITY;    // moving away from each other

    double neg_b = -2.0 * dot_prod;
    double disc = neg_b*neg_b - 4.0 * dv2 * (dist2 - sr2);

    if (disc < 0.0) return INFINITY;        // collision is imaginary

    double t = (neg_b - sqrt(disc)) / (2.0 * dv2);

    if (t <= 0.0) return INFINITY;          // collision happened last frame
    else return t;
  }

  void collide(Pod& other) {
    double m1 = mass(), m2 = other.mass();
    double mcoeff = (m1 * m2) / (m1 + m2);

    Vec2_xy d_pos = pos - other.pos;
    Vec2_xy d_v = vel - other.vel;

    Vec2_xy force = d_pos * (mcoeff * dot(d_pos, d_v) / d_pos.r2());

    Vec2_xy f1 = force * (1.0 / m1), f2 = force * (1.0 / m2);

    vel -= f1;
    other.vel += f2;

    double impulse = force.r();
    if (impulse < 120.0) {
      force *= 120.0 / impulse;
    }

    vel -= f1;
    other.vel += f2;
  }
};


struct Field {
  int num_laps;
  int num_checkpoints;
  vector<Checkpoint> checkpoints;
};

Field map;


struct PodCollision {
  Pod *p1 = nullptr, *p2 = nullptr;
  double time = INFINITY;
};
struct CheckpointCollision {
  Pod *pod = nullptr;
  double time = INFINITY;
};


struct GameState {
  union {
    array<Pod, 4> pods;
    struct {
      array<Pod, 2> my_pods;
      array<Pod, 2> their_pods;
    };
  };

  void simFrame() {
    double t = 0.0;
    do {
      // detect pod-on-pod collisions
      PodCollision pod_col;
      double col_time;
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

  GameState next() const {
    GameState ret = *this;
    ret.simFrame();
    return ret;
  }
};


int main()
{
  ios::sync_with_stdio(false);
  
  cin >> map.num_laps; cin.ignore();
  int checkpoint_count;
  cin >> map.num_checkpoints; cin.ignore();
  map.checkpoints.reserve(checkpoint_count);
  for (int i = 0; i < checkpoint_count; i++) {
    int checkpoint_x;
    int checkpoint_y;
    cin >> checkpoint_x >> checkpoint_y; cin.ignore();
    map.checkpoints.push_back({ checkpoint_x, checkpoint_y });
  }

  // game loop
  while (true) {
    for (int i = 0; i < 2; i++) {
      int x; // x position of your pod
      int y; // y position of your pod
      int vx; // x speed of your pod
      int vy; // y speed of your pod
      int angle; // angle of your pod
      int next_check_point_id; // next check point id of your pod
      cin >> x >> y >> vx >> vy >> angle >> next_check_point_id; cin.ignore();
    }
    for (int i = 0; i < 2; i++) {
      int x_2; // x position of the opponent's pod
      int y_2; // y position of the opponent's pod
      int vx_2; // x speed of the opponent's pod
      int vy_2; // y speed of the opponent's pod
      int angle_2; // angle of the opponent's pod
      int next_check_point_id_2; // next check point id of the opponent's pod
      cin >> x_2 >> y_2 >> vx_2 >> vy_2 >> angle_2 >> next_check_point_id_2; cin.ignore();
    }

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;


    // You have to output the target position
    // followed by the power (0 <= thrust <= 100)
    // i.e.: "x y thrust"
    cout << "8000 4500 100" << endl;
    cout << "8000 4500 100" << endl;
  }
}