#include <vector>
#include <string>
#include "raylib.h"
#include "../libs/imgui/imgui.h"
#include "../libs/rlImGui/rlImGui.h"

// Headers
#include "Mutation.h"
#include "Helpers.h"
#include "Globals.h"
#include "Drawing.h"
#include "Plant.h"
#include "Seed.h"
#include "Input.h"
#include "Update.h"
#include "Background.h"

int main()
{
    // 1. Window Init
    InitWindow(0, 0, "PlantSim");
    SetExitKey(KEY_F10);
    SetTargetFPS(60);
    rlImGuiSetup(true);

    // Icon
    std::string iconPath = getAssetPath("gfx/icon.png");
    Image icon = LoadImage(iconPath.c_str());
    if (icon.data != nullptr) {
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
    
    // Disable imgui.ini creation
    ImGui::GetIO().IniFilename = nullptr;

    // Globals setup
    screenWidth  = GetScreenWidth();
    screenHeight = GetScreenHeight();
    gridCols     = screenWidth / CELL_SIZE;
    gridRows     = (screenHeight / CELL_SIZE) - SOIL_LEVEL;
    gridRowsFull = gridRows + SOIL_LEVEL;

    occupied.assign(gridCols, std::vector<bool>(gridRowsFull, false));

    double simTimer = 0.0;
    requestSettingsCollapseToggle = true;
    requestGenomeCollapseToggle = true;

    // Init background video (only for linux version)
    // as I didn't get ffmpeg to work on windows yet.
    #ifdef __linux__ 
    const std::string bgPath = getAssetPath("gfx/background.mov");
    if (!InitVideoBackground(bgPath)) { TraceLog(LOG_WARNING, "Main: Video background failed to initialize."); }
    #endif

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        ImGuiIO& io = ImGui::GetIO();

        handleGlobalKeys(io);
        handleWorldInput(io);

        updateSimulation(dt, simTimer);
        drawFrame();
    }


    // Shut down
    
    // Unload the background video
    // (only on linux)
    #ifdef __linux__ 
    ShutdownVideoBackground();
    #endif
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
