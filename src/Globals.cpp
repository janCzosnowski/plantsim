#include <random>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "Genome.h"
#include "Globals.h"

// ==========================
// ===== SIMULATION CONFIG ==== 
// ==========================
int   LIFE_STEPS       = 1000;
float MAX_LIFE_TIME    = 20.0f;
float STARTING_ENERGY  = 60.0f;
float BASE_CELL_COST   = 2.0f;
float SEED_CELL_COST   = 800.0f;
float UPDATE_TIME      = 0.02f;
int exampleMaxCells    = 2000;

int CELL_SIZE          = 4;
int SOIL_LEVEL         = 30;
int TYPE_SEED          = 255;

float MUTATION_RATE    = 0.05f;
int MAX_CELL_MUTATION  = 10;

float ENERGY_HEIGHT_SCALE  = 2.0f;
float ENERGY_SOIL_PENALTY = 0.3f;
int ENERGY_ABOVE_BLOCK     = 10;
float ENERGY_MIN           = 0.01f;

float ZOOM_SPEED          = 1.1f;
float MIN_ZOOM            = 0.4f;
float MAX_ZOOM            = 10.0f;

// ==========================
// ===== WINDOW & GRID ====== 
// ==========================
int screenWidth, screenHeight;
int gridCols, gridRows, gridRowsFull;
float xOffset = 0.0f, yOffset = 0.0f;
float zoom = 1.0f;

// ==========================
// ===== SIMULATION STATE ==== 
// ==========================
bool simulationPaused = false;
bool renderEnabled = true;

// ==========================
// ===== RANDOM UTILS ======= 
// ==========================
std::mt19937 rng(12345); // deterministic seed

// ==========================
// ===== ENTITIES ===========
std::vector<Plant*> plants;
std::vector<Seed*> seeds;
std::vector<std::vector<bool>> occupied;



// ==========================
// ======= EXAMPLE GENOME ==========
std::unordered_map<int, ConditionalGenomeRule> exampleGenome = {
    {0, ConditionalGenomeRule(DirectionRule(0), DirectionRule(0), DirectionRule(1), DirectionRule(0))},
    {1, ConditionalGenomeRule(DirectionRule(0), DirectionRule(0), DirectionRule(1,2,{HEIGHT_LESS,10}), DirectionRule(0))},
    {2, ConditionalGenomeRule(DirectionRule(3,0,{HEIGHT_GREATER,5}), DirectionRule(4,0,{HEIGHT_GREATER,5}), DirectionRule(5,0,{HEIGHT_LESS,40}), DirectionRule(0))},
    {3, ConditionalGenomeRule(DirectionRule(3,0,{CELLCOUNT_LESS,180}), DirectionRule(0), DirectionRule(0), DirectionRule(255,0,{CELLCOUNT_GREATER,80}))},
    {4, ConditionalGenomeRule(DirectionRule(0), DirectionRule(4,0,{CELLCOUNT_LESS,180}), DirectionRule(0), DirectionRule(255,0,{CELLCOUNT_GREATER,80}))},
    {5, ConditionalGenomeRule(DirectionRule(0), DirectionRule(0), DirectionRule(2,0,{HEIGHT_LESS,40}), DirectionRule(0))}
};


// ==================================================
// ================== PLANT MENU ====================
// ==================================================
// GUI STATE definitions

Plant* selectedPlant = nullptr;
Seed* selectedSeed = nullptr;
bool   showEntityMenu      = false;
Vector2 menuPos      = { 0, 0 };

bool showGenomeMenu   = true;
bool genomeCollapsed   = false;
bool requestGenomeCollapseToggle   = false;
Vector2 genomeMenuPos    = { 149, 0 };
std::vector<std::string> genomeLines;
int  genomeScroll = 0;
char genomeBuffer[16384] = { 0 };

bool showSettingsMenu   = true;
bool settingsCollapsed = false;
bool requestSettingsCollapseToggle = false;
Vector2 settingsMenuPos = { 749, 0 };

bool showInfoWindow = true;
bool requestInfoCollapseToggle = false;

bool showCellProber = false;
Vector2 proberPos = { 400, 300 };
int probedCellType = -1;

bool showSolarOverlay = false;
bool showPlantEnergyOverlay = false;
Vector2 plantEnergyOverlayPos = { 0, 0 };

bool isPanning = false;
Vector2 lastMousePos = {0, 0};

bool drawOneFrame = false;
bool stepOnce = false;


// ==========================
// ======== RANDOM ==========
std::random_device rd;
std::uniform_int_distribution<int> maxCellMut(-MAX_CELL_MUTATION,MAX_CELL_MUTATION);
std::uniform_real_distribution<float> uniDist(0.f,1.f);
std::uniform_int_distribution<int> cellTypeDist(0,255);
std::uniform_int_distribution<int> condTypeDist(0,4); // NONE + 4 conditional types
std::uniform_int_distribution<int> condValueDist(1,30);



bool debugFreezeGrowth = false;
bool debugInfiniteEnergy = false;
bool debugHighlightPlant = false;


bool showStartScreen = true;
