#include <algorithm>
#include "Drawing.h"
#include "raylib.h"
#include "CellColor.h"
#include "Globals.h"
#include "Genome.h"
#include "Helpers.h"
#include "Plant.h"
#include "Seed.h"
#include "Gui.h"
#include "Background.h"

#include "../libs/imgui/imgui.h"
#include "../libs/rlImGui/rlImGui.h"


std::unordered_map<uint8_t, Color> CELL_COLORS = {
    {0, {10,10,10,255}}, {1,{0,220,50,255}}, {2,{0,180,40,255}},
    {3,{20,240,70,255}}, {4,{0,200,60,255}}, {5,{0,140,50,255}},
    {6,{10,230,50,255}}, {7,{0,160,70,255}}, {8,{0,210,80,255}},
    {9,{0,190,55,255}}, {10,{15,225,65,255}}, {255, ORANGE}
};

Color cellTypeColor(int value) {
    if(CELL_COLORS.find(value) != CELL_COLORS.end()) return CELL_COLORS[value];
    uint8_t r = (value*97)%200 + 30;
    uint8_t g = (value*57)%200 + 30;
    uint8_t b = (value*157)%200 + 30;
    return Color{r,g,b,255};
}
 
void drawGrid() {
    for(int y = 0; y <= gridRows; y++)
        DrawLine(xOffset, yOffset + y*CELL_SIZE*zoom,
                 xOffset + gridCols*CELL_SIZE*zoom, yOffset + y*CELL_SIZE*zoom, {220,220,220,255});
    for(int x = 0; x <= gridCols; x++)
        DrawLine(xOffset + x*CELL_SIZE*zoom, yOffset,
                 xOffset + x*CELL_SIZE*zoom, yOffset + gridRows*CELL_SIZE*zoom, {220,220,220,255});
}

void drawSoil() {
    DrawRectangle(xOffset, yOffset + gridRows*CELL_SIZE*zoom,
                  gridCols*CELL_SIZE*zoom, SOIL_LEVEL*CELL_SIZE*zoom, GREEN);
}

void drawSeeds() {
    for(auto* s: seeds)
        DrawRectangle(s->x*CELL_SIZE*zoom+xOffset,
                      s->y*CELL_SIZE*zoom+yOffset,
                      CELL_SIZE*zoom-1, CELL_SIZE*zoom-1, BLACK);
}

void drawPlants() {
    for(auto* p: plants)
        for(auto& c: p->cells)
            DrawRectangle(c.x*CELL_SIZE*zoom+xOffset,
                          c.y*CELL_SIZE*zoom+yOffset,
                          CELL_SIZE*zoom-1, CELL_SIZE*zoom-1,
                          cellTypeColor(c.value));
}

void drawFrame()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    rlImGuiBegin();

    if (showStartScreen)
    {
        // Only draw the start screen
        drawStartScreen();
    }
    else
    {
        // ---- World rendering (expensive) ----
        if (renderEnabled || drawOneFrame)
        {
            drawSoil();
            drawSeeds();
            drawPlants();
            drawGrid();
            
            drawOneFrame = false;
        }

        // ---- GUI ----
        drawInfo();
        drawPlantMenu();
        drawSeedMenu();
        drawGenomeMenu();
        drawSettingsMenu();
        drawCellProber();
        drawSolarOverlay();
        drawPlantEnergyOverlay();
    }

    rlImGuiEnd();
    EndDrawing();
}
