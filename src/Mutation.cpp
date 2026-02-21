#include <random>
#include <algorithm>
#include "Mutation.h"
#include "Globals.h"



int clampInt(int v, int lo, int hi) {
    return std::max(lo, std::min(v, hi));
}

DirectionRule& pickDirection(ConditionalGenomeRule& r)
{
    switch (std::uniform_int_distribution<int>(0, 3)(rng))
    {
        case 0:  return r.left;
        case 1:  return r.right;
        case 2:  return r.up;
        default: return r.down;
    }
}

std::vector<ConditionalGenomeRule> mutateGenome(const std::vector<ConditionalGenomeRule>& g)
{
    std::vector<ConditionalGenomeRule> out = g;
    if (out.empty()) return out;

    std::poisson_distribution<int> mutationBudget(std::max(0.0f, MUTATION_RATE * float(out.size())));
    int mutations = mutationBudget(rng);

    for (int i = 0; i < mutations; ++i) {
        auto& rule = out[std::uniform_int_distribution<int>(0, int(out.size()) - 1)(rng)];
        auto& dir = pickDirection(rule);  // NOW dir is DirectionRule&

        float r = uniDist(rng);

        if (r < 0.60f) {
            int delta = std::uniform_int_distribution<int>(-3, 3)(rng);
            if (uniDist(rng) < 0.1f)
                delta = std::uniform_int_distribution<int>(-12, 12)(rng);
            if (delta != 0)
                dir.condition.value = clampInt(dir.condition.value + delta, 0, 255);
        }
        else if (r < 0.80f) {
            dir.condition.type = static_cast<ConditionalType>(condTypeDist(rng));
        }
        else if (r < 0.92f) {
            dir.elseCellType = cellTypeDist(rng);
        }
        else {
            dir.ifCellType = cellTypeDist(rng);
        }
    }

    if (uniDist(rng) < MUTATION_RATE * 0.05f) {
        auto& r = out[std::uniform_int_distribution<int>(0, int(out.size()) - 1)(rng)];
        r.right = r.left; // preserve symmetry
    }

    return out;
}
