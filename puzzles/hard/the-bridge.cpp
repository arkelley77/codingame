#include <iostream>
#include <string>
#include <vector>

using namespace std;

int num_bikes;
int min_survival;
vector<bool> safe[4];

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

State global_state;

void intakeGame() {
  cin >> num_bikes;
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
  global_state.bikes.reserve(num_bikes);
  cin >> global_state.speed;
  for (int i = 0; i < num_bikes; ++i) {
    int x, y;
    bool active;
    cin >> x >> y >> active;
    if (active) global_state.bikes.push_back({ x, y });
  }
}

struct Node {
  State state;
  int num_turns;
  Node* prev_node;

  Node() : state(global_state), num_turns(0), prev_node(nullptr) {}
  Node(Node* prev, Action action)
    : state(prev->state), num_turns(prev->num_turns), prev_node(prev) {}

};

int main() {
  intakeGame();
  intakeState();
  
}