#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

struct Node {
  int idx;
  bool is_exit;
  vector<Node*> connections;
  Node* explored_from;
  float distance;
  Node(size_t idx, bool is_gateway = false) : idx(idx), is_exit(is_gateway), connections(), explored_from(nullptr), distance(0.f) {}
  int numExits() const {
    if (is_exit) return 0;
    int num_exits = 0;
    for (const Node* node : connections) { if (node->is_exit) ++num_exits; }
    return num_exits;
  }
  int numChances() const {
    if (is_exit) return 1 + explored_from->numChances();
    else if (explored_from == nullptr) return 1 - numExits();
    else return 1 + explored_from->numChances() - numExits();
  }
};
class MoreChances {
public:
  bool operator()(Node* lhs, Node* rhs) { return lhs->numChances() > rhs->numChances(); }
};
struct Connection {
  Node& node1;
  Node& node2;
  Connection(Node& node1, Node& node2) : node1(node1), node2(node2) {}
  friend ostream& operator<<(ostream& os, const Connection& c) {
    return os << c.node1.idx << ' ' << c.node2.idx;
  }
};

class Graph {
public:
  Graph() : nodes() {}

  friend istream& operator>>(istream& is, Graph& graph) {
    int num_nodes, num_links, num_exits;
    is >> num_nodes >> num_links >> num_exits; is.ignore();
    graph.nodes.reserve(num_nodes);
    for (int idx = 0; idx < num_nodes; ++idx) {
      graph.nodes.push_back(Node(idx));
    }
    for (int i = 0; i < num_links; ++i) {
      int node1, node2;
      is >> node1 >> node2; is.ignore();
      graph.nodes[node1].connections.push_back(&graph.nodes[node2]);
      graph.nodes[node2].connections.push_back(&graph.nodes[node1]);
      //graph.connections.push_back({graph.nodes[node1], graph.nodes[node2]});
    }
    for (int i = 0; i < num_exits; ++i) {
      int exit_idx;
      is >> exit_idx; is.ignore();
      graph.nodes[exit_idx].is_exit = true;
    }
    return is;
  }

  void reset() {
    for (Node& node : nodes) {
      node.explored_from = nullptr;
      node.distance = INFINITY;
    }
  }

  Connection getNextSever(int bob_idx) {
    reset();
    Node* home = &nodes[bob_idx];
    home->distance = 0;
    priority_queue<Node*, vector<Node*>, MoreChances> to_explore;
    to_explore.push(home);
    Node* node;
    Node* to_cut = nullptr;
    do {
      node = to_explore.top();
      cerr << "exploring " << hex << node << ", " << dec << node->is_exit << endl;
      to_explore.pop();
      if (node->is_exit) {
        cerr << "it's an exit! " << node->explored_from->numExits() << " exist and ";
        cerr << node->numChances() << " chances to cut" << endl;
        if (to_cut == nullptr) to_cut = node;
        else if (node->explored_from == home) {
          to_cut = node;
          break;
        }
        else if (node->numChances() < to_cut->numChances()) to_cut = node;
      }
      else {
        cerr << "Finding more nodes to explore ";
        for (Node* next : node->connections) {
          cerr << '.';
          if (next->explored_from == nullptr && next != home) {
            next->explored_from = node;
            next->distance = node->distance + 1.f;
            to_explore.push(next);
            cerr << 'o';
          }
        }
        cerr << endl;
      }
      cerr << to_explore.size() << " nodes left to explore" << endl;
    } while (!to_explore.empty());
    cerr << "cutting " << hex << to_cut << endl;
    return Connection(*to_cut, *(to_cut->explored_from));
  }

  void cut(const Connection& to_sever) {
    Node& node1 = to_sever.node1;
    vector<Node*>& conns1 = node1.connections;
    Node& node2 = to_sever.node2;
    vector<Node*>& conns2 = node2.connections;
    conns1.erase(find(conns1.begin(), conns1.end(), &node2));
    conns2.erase(find(conns2.begin(), conns2.end(), &node1));
  }

private:
  vector<Node> nodes;
  //vector<Connection> connections;
};

int main()
{
  Graph graph;
  cin >> graph;

  while (true) {
    int bob_idx;
    cin >> bob_idx; cin.ignore();
    Connection to_sever = graph.getNextSever(bob_idx);
    graph.cut(to_sever);
    cout << to_sever << endl;
  }
}
