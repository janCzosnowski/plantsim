#include <cstdint>
#include <vector>
#include <random>
#include "Genome.h"

// Implement any functions related to Genome here

// Example function implementation
ConditionalValue createConditionalValue(ConditionalType type, int value) {
    return ConditionalValue(type, value);
}

// Example function for DirectionRule
DirectionRule createDirectionRule(uint8_t ifCell, uint8_t elseCell, ConditionalValue cond) {
    return DirectionRule(ifCell, elseCell, cond);
}

// Example function for ConditionalGenomeRule
ConditionalGenomeRule createConditionalGenomeRule(DirectionRule l, DirectionRule r, DirectionRule u, DirectionRule d) {
    return ConditionalGenomeRule(l, r, u, d);
}



// =======================================
// ===== RANDOM GENOME GENERATOR ==========
ConditionalGenomeRule randomGenomeRule(std::mt19937& rng) {
    // Make 255 appear more often by duplicating it
    std::vector<uint8_t> cellOptions = {0, 1, 2, 3, 4, 5, 255, 255, 255, 255};
    std::uniform_int_distribution<int> valueDist(0, 255);

    auto randomDirectionRule = [&]() -> DirectionRule {
        ConditionalValue cond(static_cast<ConditionalType>(valueDist(rng) % 5), valueDist(rng));
        return DirectionRule(
            cellOptions[valueDist(rng) % cellOptions.size()],
            cellOptions[valueDist(rng) % cellOptions.size()],
            cond
        );
    };

    return ConditionalGenomeRule(
        randomDirectionRule(),
        randomDirectionRule(),
        randomDirectionRule(),
        randomDirectionRule()
    );
}

std::vector<ConditionalGenomeRule> generateRandomGenome(int length) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::vector<ConditionalGenomeRule> genome;
    genome.reserve(length);

    for (int i = 0; i < length; ++i) {
        genome.push_back(randomGenomeRule(rng));
    }

    return genome;
}