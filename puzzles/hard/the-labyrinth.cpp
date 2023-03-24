#include <iostream>
#include <cmath>
#include <stack>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

class Map {
public:
  enum Direction : int {
    up = 0, left = 1, down = 2, right = 3, ndirs = 4
  };
  struct GridSquare {
    int row, col;
    GridSquare(int row = -1, int col = -1) : row(row), col(col) {}
    bool discovered() const { return row > 0 && col > 0; }
  };
  struct Node {
    GridSquare location;
    char type;
    Node* connections[ndirs];
    Node* explored_from;
    float heuristic_cost, path_cost;
    Node(int row, int col)
      : location(row, col), type('?'), connections{ nullptr, nullptr, nullptr, nullptr }
      , explored_from{ nullptr }, heuristic_cost{ 0.f }, path_cost{ INFINITY } {}
    Node(const GridSquare& location = GridSquare(), char type = '?')
      : location(location), type{ type }, connections{ nullptr, nullptr, nullptr, nullptr }
      , explored_from{ nullptr }, heuristic_cost{ 0.f }, path_cost{ INFINITY } {}
    Direction getDir(Node* to) {
      for (size_t d = 0; d < 4; ++d) {
        if (to == connections[d]) return (Direction)d;
      }
      return ndirs;
    }
  };
  class NodeCompare {
  public:
    bool operator()(Node* lhs, Node* rhs) {
      return (lhs->path_cost + lhs->heuristic_cost) > (rhs->path_cost + rhs->heuristic_cost);
    }
  };
  using Row = vector<Node*>;
  using Grid = vector<Row>;

  Map(size_t rows, size_t cols)
    : grid(rows, Row(cols, nullptr)), width(cols)
    , height(rows), spawn_point(), control_room() {
    for (size_t row_idx = 0; row_idx < rows; ++row_idx) {
      Row& row = grid[row_idx];
      for (size_t col_idx = 0; col_idx < cols; ++col_idx) {
        row[col_idx] = new Node(row_idx, col_idx);
      }
    }
    for (size_t row_idx = 0; row_idx < rows; ++row_idx) {
      Row& row = grid[row_idx];
      for (size_t col_idx = 0; col_idx < cols; ++col_idx) {
        Node* node = row[col_idx];
        node->connections[up] = (row_idx > 0) ? grid[row_idx - 1][col_idx] : nullptr;
        node->connections[left] = (col_idx > 0) ? grid[row_idx][col_idx - 1] : nullptr;
        node->connections[down] = (row_idx < rows - 1) ? grid[row_idx + 1][col_idx] : nullptr;
        node->connections[right] = (col_idx < cols - 1) ? grid[row_idx][col_idx + 1] : nullptr;
      }
    }
  }

  ~Map() {
    for (Row& row : grid) {
      for (Node*& node : row) {
        delete node;
      }
    }
  }

  friend istream& operator>>(istream& is, Map& map) {
    for (int row_idx = 0; row_idx < map.height; ++row_idx) {
      Row& row = map.grid[row_idx];
      for (int col_idx = 0; col_idx < map.width; ++col_idx) {
        Node* node = row[col_idx];
        is >> node->type;
        switch (node->type) {
        case 'T':
          map.spawn_point = { row_idx, col_idx };
          break;
        case 'C':
          map.control_room = { row_idx, col_idx };
          break;
        default:
          break;
        }
      }
    }
    return is;
  }
  friend ostream& operator<<(ostream& os, const Map& map) {
    for (const Row& row : map.grid) {
      for (const Node* node : row) {
        os << node->type;
      }
      os << '\n';
    }
    return os;
  }

  void setUpGraph(const GridSquare& target = GridSquare()) {
    for (Row& row : grid) {
      for (Node*& node : row) {
        node->heuristic_cost = (target.discovered()) ? l1_dist(node->location, target) : INFINITY;
        node->path_cost = INFINITY;
        node->explored_from = nullptr;
      }
    }
  }

  void print(Node* looking_at = nullptr, Node* target_square = nullptr) const {
    for (const Row& row : grid) {
      for (const Node* node : row) {
        if (node == looking_at) cerr << '@';
        else if (node == target_square) cerr << 'X';
        else if (node->explored_from != nullptr && node->type == '.') cerr << 'o';
        else cerr << node->type;
      }
      cerr << '\n';
    }
  }

  Direction explore(GridSquare from) {
    setUpGraph();
    queue<Node*> to_explore;
    Node* home = grid.at(from.row).at(from.col);
    home->explored_from = home;
    to_explore.push(home);
    Node* node;
    while (!to_explore.empty()) {
      node = to_explore.front();
      to_explore.pop();
      if (node->type == '?') break;
      else if (node->type != '#' && node->type != 'C') {
        for (int d = 0; d < ndirs; ++d) {
          if (node->connections[d]->type == '#') continue;
          if (node->connections[d]->explored_from == nullptr) {
            node->connections[d]->explored_from = node;
            to_explore.push(node->connections[d]);
          }
        }
      }
    }
    if (node->type != '?') return ndirs;
    while (node->explored_from != home) node = node->explored_from;
    for (size_t d = 0; d < ndirs; ++d) {
      if (home->connections[d] == node) return (Direction)d;
    }
    return ndirs;
  }

  static int l1_dist(GridSquare from, GridSquare to) {
    return abs(from.col - to.col) + abs(from.row - to.row);
  }

  stack<Direction> a_star(GridSquare from, GridSquare target) {
    setUpGraph(target);
    priority_queue<Node*, vector<Node*>, NodeCompare> to_explore;
    stack<Direction> movements;

    Node* start_square = grid.at(from.row).at(from.col);
    Node* target_square = grid.at(target.row).at(target.col);
    start_square->path_cost = 0.f;
    to_explore.push(start_square);
    Node* node;
    while (!to_explore.empty()) {
      node = to_explore.top();
      to_explore.pop();
      if (node == target_square) print(node, target_square);
      if (node == target_square) break;
      else if (node->type != '#' && node->type != '?') {
        for (size_t d = 0; d < ndirs; ++d) {
          Node* next = node->connections[d];
          if (node->path_cost + 1.f < next->path_cost) {
            next->path_cost = node->path_cost + 1.f;
            next->explored_from = node;
            to_explore.push(next);
          }
        }
      }
    }
    while (node->explored_from != nullptr) {
      Direction d = node->explored_from->getDir(node);
      movements.push(d);
      node = node->explored_from;
    }
    cerr << endl;
    return movements;
  }

private:
  Grid grid;
  size_t width, height;
public:
  GridSquare spawn_point;
  GridSquare control_room;
};

int main()
{
  int nrows; // number of rows.
  int ncols; // number of columns.
  int alarm; // number of rounds between the time the alarm countdown is activated and the time the alarm goes off.
  cin >> nrows >> ncols >> alarm; cin.ignore();

  Map map(nrows, ncols);

  stack<Map::Direction> movements;
  Map::Direction explore_dir = Map::up;

  // game loop
  while (true) {
    int rick_row; // row where Rick is located.
    int rick_col; // column where Rick is located.
    cin >> rick_row >> rick_col; cin.ignore();
    cin >> map;
    cerr << map;

    if (explore_dir != Map::ndirs) explore_dir = map.explore({ rick_row, rick_col });

    if (explore_dir != Map::ndirs) {
      movements.push(explore_dir);
    }
    else {
      cerr << "Done exploring" << endl;
      if (movements.empty()) {
        if (rick_row == map.control_room.row && rick_col == map.control_room.col) {
          // we've just gotten to the control room
          // time to SCRAM!
          movements = move(map.a_star({ rick_row, rick_col }, map.spawn_point));
        }
        else {
          // navigate to the control room
          movements = map.a_star({ rick_row, rick_col }, map.control_room);
        }
      }
    }
    if (!movements.empty()) {
      switch (movements.top()) {
      case Map::up:
        cout << "UP" << endl;
        break;
      case Map::left:
        cout << "LEFT" << endl;
        break;
      case Map::down:
        cout << "DOWN" << endl;
        break;
      case Map::right:
        cout << "RIGHT" << endl;
        break;
      default:
        break;
      }
      movements.pop();
    }
    else break;
  }
}
