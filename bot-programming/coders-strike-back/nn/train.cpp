#include "sim.h"
#include "nn.h"
#include "genetic.h"

#include <algorithm>
#include <fstream>

using namespace std;

vector<pair<Field, SimState>> races;

void loadRaces() {
  for (int field_id = 1; field_id <= 15; ++field_id) {
    string fname = string("c:/Users/arkel/source/repos/arkelley77/codingame/bot-programming/coders-strike-back/nn/fields/") + to_string(field_id) + string(".txt");
    ifstream ifs(fname);
    Field field;
    SimState state;
    ifs >> field >> state;
    races.push_back(make_pair(field, state));
    ifs.close();
  }
}

double compete(Agent& agent1, Agent& agent2) {
  double score = 0.f;
  for (int i = 0; i < 5; ++i) {
    auto& race = races[randint(1, 15) - 1];
    Field& field = race.first;
    SimState state = race.second;

    do {
      array<Move, 2> move1 = agent1.race(field, state, true);
      array<Move, 2> move2 = agent2.race(field, state, false);

      for (size_t i = 0; i < 2; ++i) state.pods[0 + i].apply(move1[i]);
      for (size_t i = 0; i < 2; ++i) state.pods[2 + i].apply(move2[i]);

      state.simFrame(field);
    } while (state.outcome(field) == 0);

    // completion percentage
    double completion = 0.0;
    for (size_t i = 0; i < 4; ++i) {
      double sign = (i / 2 == 0) ? 1.0 : -1.0;
      if (state.pods[i].laps > 0) completion += sign * state.pods[i].laps * field.num_checkpoints;
      if (state.pods[i].next_cp > 0) completion += sign * (float)state.pods[i].next_cp;
    }
    completion /= field.num_checkpoints * field.num_laps;
    score += round(completion * 10000.0);  // completion weight

    if (completion == 0.0) {
      for (size_t i = 0; i < 4; ++i) {
        double sign = (i / 2 == 0) ? 1.0 : -1.0;
        double dist2cp = (state.pods[i].pos - field.checkpoints[state.pods[i].next_cp]).r();
        int prev_cp = state.pods[i].next_cp - 1;
        if (prev_cp < 0) prev_cp = field.num_checkpoints - 1;
        double cp2cp = (field.checkpoints[prev_cp] - field.checkpoints[state.pods[i].next_cp]).r();
        score += sign * cp2cp / dist2cp;
      }
    }
  }

  return score / 5.0;
}

bool operator>(Genome& a, Genome& b) {
  return compete(agent(a), agent(b)) > 0;
}

class MergeSort {
  vector<Genome> aux;
  vector<Genome> *in, *out;
public:
  vector<Genome> sort(vector<Genome>& arr) {
    if (arr.size() <= 1) return arr;
    aux = vector<Genome>(arr.size());
    in = &arr;
    out = &aux;
    size_t half = 1;
    while (half < arr.size()) {
      size_t chunk_sz = half * 2;
      size_t chunk = 0;
      do {
        size_t mid = chunk + half;
        if (mid >= arr.size()) {
          do {
            (*out)[chunk]
              = (*in)[chunk];
            ++chunk;
          } while (chunk < arr.size());
          break;
        }
        size_t end = min(
          chunk + chunk_sz,
          arr.size()
        );
        merge(chunk, end, mid);
        chunk = end;
      } while (chunk < arr.size());
      swap(in, out);
      half = chunk_sz;
    }
    return *in;
  }
  
private:
  void merge(size_t ptr, size_t end, size_t mid) {
    size_t iL = ptr;
    size_t iR = mid;
    while (ptr < end) {
      if (iL >= mid) {
        (*out)[ptr++] = (*in)[iR++];
      }
      else if (iR >= end) {
        (*out)[ptr++] = (*in)[iL++];
      }
      else {
        if ((*in)[iL] > (*in)[iR]) {
          (*out)[ptr++] = (*in)[iL++];
        } else {
          (*out)[ptr++] = (*in)[iR++];
        }
      }
    }
  }
};

Genome& train(size_t population_size, size_t num_generations) {
  vector<Genome> population = createPopulation(population_size);
  MergeSort sorter;

  sorter.sort(population);
  cout << "Gen " << 0 << ", best: ";
  cout << compete(agent(population[0]), agent(population[1])) << endl;
  for (size_t generation = 1; generation < num_generations; ++generation) {
    makeBabies(population, true);
    
    sorter.sort(population);

    cout << "Gen " << generation;
    cout << ", self play: " << abs(compete(agent(population[0]), agent(population[0])));
    cout << ", best 2: " << compete(agent(population[0]), agent(population[1]));
    cout << ", spread: " << compete(agent(population.front()), agent(population.back()));
    cout << endl;
  }
  return population[0];
}

int main() {
  initRandom();
  loadRaces();
  
  Genome out = train(50, 50);

  cout << "\nFinal network:\n" << endl;
  cout << "constexpr Genome net = {\n  ";
  int column = 0;
  for (auto& i : out) {
    if (++column > 8) {
      cout << "\n  ";
      column = 0;
    }
    cout << "0x" << hex << reinterpret_cast<int32_t&>(i) << ", ";
  }
  cout << "\n};" << endl;

  return 0;
}