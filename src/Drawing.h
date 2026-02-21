#pragma once
#include <vector>
#include "raylib.h" // For Color, DrawRectangle, etc.

struct Seed;
struct Cell;
struct Plant;

extern int gridRows;
extern int gridCols;
extern float xOffset;
extern float yOffset;
extern int CELL_SIZE;
extern float zoom;
extern int SOIL_LEVEL;
extern std::vector<Seed*> seeds;
extern std::vector<Plant*> plants;

// Forward declaration
Color cellTypeColor(int value);

// Drawing functions
void drawGrid();
void drawSoil();
void drawSeeds();
void drawFrame();

// Menus & overlays
void drawInfo();
void drawGenomeMenu();
void drawSettingsMenu();
void drawCellProber();
void openGenomeMenu();
void drawPlantMenu();

void drawPlantEnergyOverlay();
void drawSolarOverlay();

void drawFrame();