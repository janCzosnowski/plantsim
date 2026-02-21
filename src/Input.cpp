#include "Input.h"
#include "Globals.h"
#include "Helpers.h"
#include "Plant.h"
#include "Seed.h"
#include "Genome.h"

void handleZoom()
{
    float wheel = GetMouseWheelMove();
    if (wheel == 0.0f) return;

    Vector2 worldBefore = ScreenToWorld(GetMouseX(), GetMouseY());
    float scale = (wheel > 0) ? ZOOM_SPEED : 1.0f / ZOOM_SPEED;
    zoom = clamp(zoom * scale, MIN_ZOOM, MAX_ZOOM);

    xOffset = GetMouseX() - worldBefore.x * CELL_SIZE * zoom;
    yOffset = GetMouseY() - worldBefore.y * CELL_SIZE * zoom;
}

void handleMouseActions()
{
    Vector2 world = ScreenToWorld(GetMouseX(), GetMouseY());
    int mx = clamp((int)world.x, 0, gridCols - 1);
    int my = clamp((int)world.y, 0, gridRows - 1);

    // Right click: select plant
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        // Reset current selection
        selectedPlant = nullptr;
        selectedSeed = nullptr;
        showEntityMenu = false;

        // Check plants first
        for (auto* p : plants)
        {
            for (auto& c : p->cells)
            {
                if (c.x == mx && c.y == my)
                {
                    selectedPlant = p;
                    if (p->alive)
                    {
                        showEntityMenu = true;
                        menuPos = { (float)GetMouseX(), (float)GetMouseY() };
                    }
                    break;
                }
            }
            if (showEntityMenu) break;
        }

        // If no plant was clicked, check seeds
        if (!showEntityMenu)
        {
            for (auto* s : seeds)
            {
                if (s->x == mx && s->y == my)
                {
                    selectedSeed = s;
                    showEntityMenu = true;
                    menuPos = { (float)GetMouseX(), (float)GetMouseY() };
                    break;
                }
            }
        }
    }

// ---------- LEFT CLICK: place seed ----------
    if (!showEntityMenu && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !occupied[mx][my])
    {
        std::vector<ConditionalGenomeRule> genomeVec;
        genomeVec.reserve(exampleGenome.size());
        for (int i = 0; i < (int)exampleGenome.size(); i++)
            genomeVec.push_back(exampleGenome[i]);

        seeds.push_back(new Seed(mx, my, genomeVec, exampleMaxCells, false));
    }

    // Left click: cell prober
    if (showCellProber && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        probedCellType = 0; // default empty

        for (auto* s : seeds)
            if (s->x == mx && s->y == my) probedCellType = TYPE_SEED;

        for (auto* p : plants)
        {
            for (auto& c : p->cells)
            {
                if (c.x == mx && c.y == my)
                {
                    probedCellType = c.value;
                    break;
                }
            }
            if (probedCellType != 0 && probedCellType != TYPE_SEED) break;
        }
    }
}

void handleCameraPan()
{
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON))
    {
        isPanning = true;
        lastMousePos = { (float)GetMouseX(), (float)GetMouseY() };
    }

    if (IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON))
    {
        isPanning = false;
    }

    if (!isPanning) return;

    Vector2 mouseNow = { (float)GetMouseX(), (float)GetMouseY() };
    Vector2 delta = { mouseNow.x - lastMousePos.x, mouseNow.y - lastMousePos.y };

    xOffset += delta.x;
    yOffset += delta.y;

    lastMousePos = mouseNow;
}

void handleGlobalKeys(ImGuiIO& io)
{
    if (io.WantCaptureKeyboard) return; // <-- skip global keys when typing in ImGui

    if (IsKeyPressed(KEY_G)) requestGenomeCollapseToggle = true;
    if (IsKeyPressed(KEY_S)) requestSettingsCollapseToggle = true;
    if (IsKeyPressed(KEY_I)) requestInfoCollapseToggle = true;
    if (IsKeyPressed(KEY_SPACE)) simulationPaused = !simulationPaused;
    if (IsKeyPressed(KEY_R)) renderEnabled = !renderEnabled;
    if (IsKeyPressed(KEY_P)) showCellProber = !showCellProber;
    if (IsKeyPressed(KEY_ONE)) showSolarOverlay = !showSolarOverlay;
    if (IsKeyPressed(KEY_TWO)) showPlantEnergyOverlay = !showPlantEnergyOverlay;
    if (IsKeyPressed(KEY_F)){ simulationPaused = true; stepOnce = true; drawOneFrame = true; }
    if (IsKeyPressed(KEY_L))
    {
        // Convert mouse position to world coordinates
        Vector2 world = ScreenToWorld(GetMouseX(), GetMouseY());
        int mx = clamp((int)world.x, 0, gridCols - 1);
        int my = clamp((int)world.y, 0, gridRows - 1);

        if (!occupied[mx][my])
        {
            // Generate a random 5-gene genome
            std::vector<ConditionalGenomeRule> genome = generateRandomGenome(25);

            // Use exampleMaxCells or any other value you want
            seeds.push_back(new Seed(mx, my, genome, exampleMaxCells, false));
        }
    }
}

void handleWorldInput(ImGuiIO& io)
{
    if (io.WantCaptureMouse || io.WantCaptureKeyboard) return;

    handleZoom();
    handleMouseActions();
    handleCameraPan();
}

