// Genome.h

#pragma once
#include <cstdint>
#include <unordered_map>

enum ConditionalType : uint8_t { NONE=0, HEIGHT_GREATER=1, HEIGHT_LESS=2, CELLCOUNT_GREATER=3, CELLCOUNT_LESS=4 };

struct ConditionalValue { 
    ConditionalType type = NONE; 
    int value = 0; 
    ConditionalValue() = default;
    ConditionalValue(ConditionalType t, int v) : type(t), value(v) {}
};

struct DirectionRule {
    uint8_t ifCellType;
    uint8_t elseCellType;
    ConditionalValue condition;

    // 3-argument constructor
    DirectionRule(uint8_t ifCellType, uint8_t elseCellType, ConditionalValue condition)
        : ifCellType(ifCellType), elseCellType(elseCellType), condition(condition) {}

    // default constructor
    DirectionRule() = default;

    // optional 1-int constructor for convenience
    DirectionRule(int x) 
        : ifCellType(x), elseCellType(x), condition(ConditionalValue(HEIGHT_LESS,0)) {} 
};

struct ConditionalGenomeRule {
    DirectionRule left, right, up, down;

    ConditionalGenomeRule() = default;
    ConditionalGenomeRule(DirectionRule l, DirectionRule r, DirectionRule u, DirectionRule d)
        : left(l), right(r), up(u), down(d) {}
};


std::vector<ConditionalGenomeRule> generateRandomGenome(int length = 5);