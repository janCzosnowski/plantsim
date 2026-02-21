#pragma once
#include <vector>

class Seed {
public:
    int x, y;
    std::vector<ConditionalGenomeRule> genome;
    int maxCells;
    bool alive = true;

    // Constructor
    Seed(int xx, int yy, std::vector<ConditionalGenomeRule> g, int maxC, bool mutate = true);

    // Public method
    void die();
};
