#include <vector>

#include "Globals.h"
#include "Genome.h"
#include "Helpers.h"
#include "Mutation.h"
#include "Seed.h"

Seed::Seed(int xx, int yy, std::vector<ConditionalGenomeRule> g, int maxC, bool mutate)
    : x(clamp(xx, 0, gridCols - 1)),
      y(clamp(yy, 0, gridRows - 1)),
      genome(mutate ? mutateGenome(g) : g),
      maxCells(std::max(1, maxC + maxCellMut(rng)))
{
    occupied[x][y] = true;
}

void Seed::die() {
    if (!alive) return;
    alive = false;

    if (x >= 0 && x < gridCols && y >= 0 && y < gridRowsFull)
        occupied[x][y] = false;

    for (int i = (int)seeds.size() - 1; i >= 0; i--) {
        if (seeds[i] == this) {
            seeds.erase(seeds.begin() + i);
            break;
        }
    }
    
    if (selectedSeed == this)
    {
        selectedSeed = nullptr;
        showEntityMenu = false;
    }
}
