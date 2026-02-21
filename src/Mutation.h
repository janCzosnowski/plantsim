#pragma once
#include <vector>
#include "Genome.h"
#include <random>

// globals (defined elsewhere)
extern std::mt19937 rng;
extern std::uniform_real_distribution<float> uniDist;
extern std::uniform_int_distribution<int> cellTypeDist;
extern std::uniform_int_distribution<int> condTypeDist;
extern std::uniform_int_distribution<int> condValueDist;
extern float MUTATION_RATE;

// helper functions
int clampInt(int v, int lo, int hi);
DirectionRule& pickDirection(ConditionalGenomeRule& r);

// main function
std::vector<ConditionalGenomeRule> mutateGenome(const std::vector<ConditionalGenomeRule>& g);
