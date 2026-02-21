#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "Genome.h"
#include "raylib.h"


// ========================
// ===== INLINE HELPERS ====
// ========================

// Trim whitespace from string
static inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// Clamp template
template <typename T>
T clamp(T v, T lo, T hi) { return (v<lo)?lo:(v>hi)?hi:v; }


// ========================
// ===== FUNCTION DECLARATIONS =====
// ========================

// Convert DirectionRule to string
std::string directionToString(const DirectionRule& d, bool addSpace=true);

// Convert ConditionalGenomeRule to string
std::string ruleToString(const ConditionalGenomeRule& r, bool addSpace=true);

// Convert entire genome map to string
std::string genomeMapToString(const std::unordered_map<int, ConditionalGenomeRule>& genome, bool addSpace=true);

// Parse a single line {…} into ConditionalGenomeRule
ConditionalGenomeRule parseGenomeLine(const std::string& line);

// Parse a labeled line "17: {…}" into pair<int,ConditionalGenomeRule>
std::pair<int, ConditionalGenomeRule> parseLabeledGenomeLine(const std::string& line);

// Parse entire labeled genome text
std::unordered_map<int, ConditionalGenomeRule> parseLabeledGenomeText(const std::string& text);


float calculateEnergyAtCell(int x, int y);
bool isPlantCell(int x,int y);
Vector2 ScreenToWorld(int sx,int sy);
void updateSimulation(float dt, double &simTimer);

extern std::string getAssetPath(const std::string& relativePath);
