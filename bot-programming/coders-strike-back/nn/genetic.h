#ifndef GENETIC_H
#define GENETIC_H

#include "sim.h"
#include "nn.h"

#include <array>
#include <cstdint>
#include <vector>

constexpr float p_crossover = 0.f;
constexpr float p_mutate = 1.f / 2048.f;

constexpr int max_mutations = 4;

void initRandom();

// Returns a random number
// a <= randint(a, b) <= b
int randint(int a, int b);

// Randomly decides whether an event
// happens based on its probability
bool eventHappens(float probability);

bool coinFlip();

typedef int32_t Chromosome;
Chromosome randomChromosome();

constexpr size_t num_chromosomes = sizeof(Agent) / sizeof(Chromosome);
typedef std::array<Chromosome, num_chromosomes> Genome;

static inline Genome& genome(Agent& a) { return reinterpret_cast<Genome&>(a); }
static inline Agent& agent(Genome& g) { return reinterpret_cast<Agent&>(g); }

void makeBabies(std::vector<Genome>& population, bool keep_original = false);
std::vector<Genome> makeBabies(const Genome& a, const Genome& b, size_t num_children, bool keep_original = false);

std::vector<Genome> createPopulation(size_t number);

#endif