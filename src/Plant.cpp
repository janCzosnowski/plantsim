#include "Plant.h"    // full Plant definition
#include "Seed.h"     // needed for creating Seed objects
#include "Globals.h"  // global vectors
#include "Helpers.h"  // ONLY if needed for helper functions
#include <algorithm>  // for std::max, std::clamp


Plant::Plant(int ox, int oy, std::vector<ConditionalGenomeRule> g, int maxC)
    : genome(g), maxCells(maxC), energy(STARTING_ENERGY)
{
    ox = clamp(ox, 0, gridCols - 1);
    oy = clamp(oy, 0, gridRows - 1);
    cells.push_back({ ox, oy, 0 });
    occupied[ox][oy] = true;
    maxLifeSteps = LIFE_STEPS;
    lifeSteps = maxLifeSteps;
}

void Plant::update() {
    collectSunlight();
    grow();
    lifeSteps--;
    if (lifeSteps <= 0) die();
}

void Plant::die(bool dropSeeds)
{
    if (!alive) return;
    alive = false;

    for (auto& c : cells)
    {
        if (c.value == TYPE_SEED && dropSeeds)
            seeds.push_back(new Seed(c.x, c.y, genome, maxCells));

        occupied[c.x][c.y] = false;
    }

    if (selectedPlant == this)
    {
        selectedPlant = nullptr;
        showEntityMenu = false;
    }
}


void Plant::grow() {
    if (energy < BASE_CELL_COST) return;
    int initial = (int)cells.size();

    for (int i = 0; i < initial; i++) {
        PlantCell c = cells[i];
        if ((int)cells.size() >= maxCells) break;
        if (c.value >= genome.size()) continue;

        ConditionalGenomeRule r = genome[c.value];
        bool cond;

        cond = checkCondition(r.left.condition);
        tryGrow(c.x - 1, c.y, cond ? r.left.ifCellType : r.left.elseCellType);

        cond = checkCondition(r.right.condition);
        tryGrow(c.x + 1, c.y, cond ? r.right.ifCellType : r.right.elseCellType);

        cond = checkCondition(r.up.condition);
        tryGrow(c.x, c.y - 1, cond ? r.up.ifCellType : r.up.elseCellType);

        cond = checkCondition(r.down.condition);
        tryGrow(c.x, c.y + 1, cond ? r.down.ifCellType : r.down.elseCellType);
    }
}

bool Plant::checkCondition(const ConditionalValue& cond) {
    int topY = gridRows;
    for (auto& c : cells)
        if (c.y < topY) topY = c.y;

    switch (cond.type) {
        case HEIGHT_GREATER: return gridRows - topY >= cond.value;
        case HEIGHT_LESS: return gridRows - topY <= cond.value;
        case CELLCOUNT_GREATER: return (int)cells.size() > cond.value;
        case CELLCOUNT_LESS: return (int)cells.size() < cond.value;
        case NONE: default: return true;
    }
}

void Plant::tryGrow(int xx, int yy, uint8_t type) {
    if (type == 0) return;
    xx = clamp(xx, 0, gridCols - 1);
    yy = clamp(yy, 0, gridRows - 1);
    if (occupied[xx][yy]) return;

    float cost = (type == TYPE_SEED) ? SEED_CELL_COST : BASE_CELL_COST;
    if (energy < cost) return;

    energy -= cost;
    cells.push_back({ xx, yy, type });
    occupied[xx][yy] = true;
}

void Plant::collectSunlight() {
    energy = std::max(energy, ENERGY_MIN);

    for (auto& c : cells) {
        float sun = calculateEnergyAtCell(c.x, c.y);
        energy += sun;
    }
}
