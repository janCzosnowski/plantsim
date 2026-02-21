#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include "raylib.h"
#include "Genome.h"

// Forward declarations only
class Seed;

// PlantCell struct
struct PlantCell { int x, y; uint8_t value; };


class Plant {
public: 
    Plant(int ox, int oy, std::vector<ConditionalGenomeRule> g, int maxC);
    void update();
    void die(bool dropSeeds = true);
    bool alive = true;
private:
    void grow();
    void collectSunlight();
    void tryGrow(int xx, int yy, uint8_t type);
    bool checkCondition(const ConditionalValue& cond);

public:
    std::vector<PlantCell> cells;
    std::vector<ConditionalGenomeRule> genome;

    int maxCells;
    int lifeSteps;
    int maxLifeSteps;
    float energy;
};
