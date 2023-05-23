#include <array>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum Action {
  SPEED = 0, SLOW = 1, JUMP = 2, WAIT = 3, UP = 4, DOWN = 5, action_count = 6
};
const string actions[action_count] = {
  "SPEED", "SLOW", "JUMP", "WAIT", "UP", "DOWN"
};

struct Bike { int x, y; };

struct State {
  int speed;
  vector<Bike> bikes;
};

int num_bikes;
int min_survival;
array<vector<bool>, 4> safe;
State global_state;

void intakeGame() {
  cin >> num_bikes;
  global_state.bikes.reserve(num_bikes);
  cin >> min_survival;
  for (int i = 0; i < 4; ++i) {
    char c;
    cin >> c;
    while (c != '\n') {
      safe[i].push_back(c == '.');
    }
    for (int j = 0; j < 10; ++j) {
      safe[i].push_back(true);
    }
  }
}

void intakeState() {
  global_state.bikes.clear();
  //global_state.bikes.reserve(num_bikes);
  cin >> global_state.speed;
  for (int i = 0; i < num_bikes; ++i) {
    int x, y;
    bool active;
    cin >> x >> y >> active;
    if (active) global_state.bikes.push_back({ x, y });
  }
}

void updateState(State& state, Action action) {
  switch (action) {
    case SPEED: if (state.speed < 50) ++state.speed; break;
    case SLOW:  if (state.speed > 0) --state.speed; break;
    
    case JUMP:  {
      for (auto it = state.bikes.begin(); it != state.bikes.end();) {
        it->x += state.speed;
        if (!safe[it->y][it->x]) it = state.bikes.erase(it);
        else ++it;
      }
      return;
    }

    case WAIT:  break;

    case UP:    {
      for (auto& bike : state.bikes) {
        if (bike.y == 0) {
          updateState(state, WAIT);
          return;
        }
      }
      for (auto it = state.bikes.begin(); it != state.bikes.end(); ++it) {
        for (int x = it->x + 1; x < min(it->x + state.speed, (int)safe[0].size());) {
          if (!safe[it->y][x] || !safe[it->y - 1][x]) {
            it = state.bikes.erase(it);
            x = it->x + 1;
          }
          else ++x;
        }
      }
    }
  }
}

struct Node {
  State state;
  int num_turns;
  Node* prev_node;

  Node() : state(global_state), num_turns(0), prev_node(nullptr) {}
  Node(Node* prev, Action action)
    : state(prev->state), num_turns(prev->num_turns), prev_node(prev) {
    updateState(state, action);
  }
};

int main() {
  intakeGame();
  intakeState();
  
}