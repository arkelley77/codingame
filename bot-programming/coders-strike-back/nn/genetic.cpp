#include "genetic.h"

#include <array>
#include <random>

using namespace std;

random_device dev;
mt19937 gen;

void initRandom() { gen.seed(dev()); }

int randint(int a, int b) { return uniform_int_distribution(a, b)(gen); }

bool eventHappens(float p) { return (static_cast<float>(gen()) / UINT32_MAX) < p; }
bool coinFlip() { return uniform_int_distribution(0, 1)(gen); }

Chromosome randomChromosome() {
  float r = uniform_real_distribution<float>(-1.f, 1.f)(gen);
  return *reinterpret_cast<int32_t*>(&r);
}

Chromosome makeBaby(Chromosome a, Chromosome b) {
  Chromosome& parent = (coinFlip()) ? a : b;
  float& parent_f = reinterpret_cast<float&>(parent);

  if (eventHappens(p_mutate)) {
    parent_f += uniform_real_distribution<float>(-0.1f, 0.1f)(gen);
    parent_f = clamp(parent_f, -1.f, 1.f);
  }

  return parent;
}

Genome makeBaby(const Genome& a, const Genome& b) {
  Genome ret;
  for (size_t i = 0; i < num_chromosomes; ++i) ret[i] = makeBaby(a[i], b[i]);
  return ret;
}

void makeBabies(vector<Genome>& population, bool keep_original) {
  Genome a = population[0];
  Genome b = population[1];
  auto it = population.begin();
  if (keep_original) it += 2;
  for (; it != population.end(); ++it) {
    *it = makeBaby(a, b);
  }
}

vector<Genome> makeBabies(const Genome& a, const Genome& b, size_t num_children, bool keep_original) {
  vector<Genome> ret;
  ret.reserve(num_children);
  for (size_t i = 0; i < num_children; ++i) ret.push_back(makeBaby(a, b));
  if (keep_original) {
    ret[0] = a;
    ret[1] = b;
  }
  return ret;
}

vector<Genome> createPopulation(size_t num_agents) {
  vector<Genome> ret(num_agents);
  for (Genome& g : ret) {
    for (Chromosome& c : g) c = randomChromosome();
  }
  return ret;
}
