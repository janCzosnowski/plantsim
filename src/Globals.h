#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include "raylib.h"
#include "Genome.h"

class Plant;
class Seed;

// ==========================
// ===== SIMULATION CONFIG ==== 
// ==========================
extern int   LIFE_STEPS;
extern float MAX_LIFE_TIME;
extern float STARTING_ENERGY;
extern float BASE_CELL_COST;
extern float SEED_CELL_COST;
extern float UPDATE_TIME;
extern int exampleMaxCells;

extern int CELL_SIZE;
extern int SOIL_LEVEL;
extern int TYPE_SEED;

extern float MUTATION_RATE;
extern int MAX_CELL_MUTATION;

extern float ENERGY_HEIGHT_SCALE;
extern float ENERGY_SOIL_PENALTY;
extern int ENERGY_ABOVE_BLOCK;
extern float ENERGY_MIN;

extern float ZOOM_SPEED;
extern float MIN_ZOOM;
extern float MAX_ZOOM;

// ==========================
// ===== WINDOW & GRID ====== 
// ==========================
extern int screenWidth, screenHeight;
extern int gridCols, gridRows, gridRowsFull;
extern float xOffset, yOffset;
extern float zoom;

// ==========================
// ===== SIMULATION STATE ==== 
// ==========================
extern bool simulationPaused;
extern bool renderEnabled;

// ==========================
// ===== RANDOM UTILS ======= 
// ==========================
extern std::mt19937 rng;


// ==========================
// ===== ENTITIES ===========
extern std::vector<Plant*> plants;
extern std::vector<Seed*> seeds;
extern std::vector<std::vector<bool>> occupied;

extern std::unordered_map<int, ConditionalGenomeRule> exampleGenome;

// ==========================
// ===== GUI STATE ==========
extern Plant* selectedPlant;
extern Seed* selectedSeed;
extern bool showEntityMenu;
extern Vector2 menuPos;
inline constexpr int menuWidth  = 150;
inline constexpr int menuHeight = 60;

extern bool showGenomeMenu;
extern bool genomeCollapsed;
extern bool requestGenomeCollapseToggle;
extern Vector2 genomeMenuPos;
inline constexpr int genomeMenuWidth  = 600;
inline constexpr int genomeMenuHeight = 269;
extern std::vector<std::string> genomeLines;
extern int genomeScroll;
inline constexpr int maxGenomeLines = 17;
extern char genomeBuffer[16384];

extern bool showSettingsMenu;
extern bool settingsCollapsed;
extern bool requestSettingsCollapseToggle;
extern Vector2 settingsMenuPos;
inline constexpr int settingsMenuWidth  = 500;
inline constexpr int settingsMenuHeight = 448;

extern bool showInfoWindow;
extern bool requestInfoCollapseToggle;

extern bool showCellProber;
extern Vector2 proberPos;
extern int probedCellType;

extern bool showSolarOverlay;
extern bool showPlantEnergyOverlay;
extern Vector2 plantEnergyOverlayPos;

extern bool isPanning;
extern Vector2 lastMousePos;

extern bool drawOneFrame;
extern bool stepOnce;


// ==========================
// ======== RANDOM ==========

extern std::random_device rd;
extern std::uniform_int_distribution<int> maxCellMut;
extern std::uniform_real_distribution<float> uniDist;
extern std::uniform_int_distribution<int> cellTypeDist;
extern std::uniform_int_distribution<int> condTypeDist;
extern std::uniform_int_distribution<int> condValueDist;


extern bool debugFreezeGrowth;
extern bool debugInfiniteEnergy;
extern bool debugHighlightPlant;


extern bool showStartScreen;
