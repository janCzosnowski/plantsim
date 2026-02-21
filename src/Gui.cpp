#include "../libs/imgui/imgui.h"
#include "../libs/rlImGui/rlImGui.h"
#include <algorithm>
#include "Globals.h"
#include "Helpers.h"
#include "Plant.h"
#include "Seed.h"
#include "Gui.h"
#include "Background.h"


void drawGenomeMenu(){
    if (!showGenomeMenu) return;

    ImGui::SetNextWindowPos(ImVec2(genomeMenuPos.x, genomeMenuPos.y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(genomeMenuWidth, genomeMenuHeight), ImGuiCond_Once);

    ImGui::Begin("Genome Editor", nullptr);

    // ---------- Keyboard Collapse Toggle ----------
    if (requestGenomeCollapseToggle)
    {
        bool collapsed = ImGui::IsWindowCollapsed();
        ImGui::SetWindowCollapsed(!collapsed, ImGuiCond_Always);
        requestGenomeCollapseToggle = false;
    }

    // ---------- Reload genome when window is restored ----------
    static bool wasCollapsed = true;
    bool isCollapsed = ImGui::IsWindowCollapsed();
    if (wasCollapsed && !isCollapsed)
    {
        std::string text = genomeMapToString(exampleGenome, true);
        memset(genomeBuffer, 0, sizeof(genomeBuffer));
        strncpy(genomeBuffer, text.c_str(), sizeof(genomeBuffer) - 1);
    }
    wasCollapsed = isCollapsed;

    // ---------- Cheatsheet ----------
    ImGui::Text("Format per cell type: left; right; up; down;");
    ImGui::Text("Format per direction: ifCellType, elseCellType, condType, condValue");
    ImGui::Text("0=NONE, 1=HEIGHT_GREATER, 2=HEIGHT_LESS, 3=CELLCOUNT_GREATER, 4=CELLCOUNT_LESS");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---------- Count lines ----------
    int lineCount = 1;
    for (size_t i = 0; genomeBuffer[i]; i++)
        if (genomeBuffer[i] == '\n') lineCount++;

    float lineHeight = ImGui::GetTextLineHeightWithSpacing();

    // ---------- Genome Input ----------
    ImGui::InputTextMultiline(
        "##genome",
        genomeBuffer,
        sizeof(genomeBuffer),
        ImVec2(-1, lineCount * lineHeight + 20),
        ImGuiInputTextFlags_AllowTabInput
    );

    ImGui::Spacing();
    ImGui::Separator();

    // ---------- Buttons ----------
    static std::string genomeError = "";
    ImGui::BeginGroup(); 
    if (ImGui::Button("Apply"))
    {
        try
        {
            auto newGenome = parseLabeledGenomeText(std::string(genomeBuffer));
            exampleGenome = newGenome;      // apply only on successful parse
            genomeError.clear();             // clear error
            ImGui::SetWindowCollapsed(true, ImGuiCond_Always);
            requestGenomeCollapseToggle = false;
        }
        catch (const std::exception& e)
        {
            genomeError = e.what();
            printf("%s\n", genomeError.c_str());
        }
    }

    // SameLine Cancel
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        ImGui::SetWindowCollapsed(true, ImGuiCond_Always);
    }

    // Error message after Cancel, same line
    ImGui::SameLine();
    if (!genomeError.empty())
    {
        ImGui::TextColored(ImVec4(1,0,0,1), "%s", genomeError.c_str());
    }

    ImGui::EndGroup();
    ImGui::End();
}


void openGenomeMenu(){
    showGenomeMenu = true;

    std::vector<std::pair<int, ConditionalGenomeRule>> genomeVector(
        exampleGenome.begin(),
        exampleGenome.end()
    );

    std::sort(
        genomeVector.begin(),
        genomeVector.end(),
        [](const auto& a, const auto& b)
        {
            return a.first < b.first;
        }
    );

    std::string genomeText;
    for (auto& [label, rule] : genomeVector)
    {
        genomeText += std::to_string(label) +
                      ": {" +
                      ruleToString(rule, true) +
                      "}\n";
    }

    memset(genomeBuffer, 0, sizeof(genomeBuffer));
    strncpy(genomeBuffer, genomeText.c_str(), sizeof(genomeBuffer) - 1);
}


void drawPlantMenu(){
    if (!showEntityMenu || selectedPlant == nullptr || !selectedPlant->alive)
        return;

    // Slightly wider window + ignore stored collapsed state
    ImGui::SetNextWindowSizeConstraints(ImVec2(180, 0), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(ImVec2(menuPos.x, menuPos.y), ImGuiCond_Appearing);

    ImGui::Begin(
        "Plant Menu",
        &showEntityMenu,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse
    );

    Plant* p = selectedPlant;

    // Info
    ImGui::Text("Energy: %.2f", p->energy);
    ImGui::Text("Cells: %d / %d", (int)p->cells.size(), p->maxCells);
    ImGui::Text("Life: %d / %d", p->lifeSteps, p->maxLifeSteps);
    ImGui::Text("Genome size: %d", (int)p->genome.size());

    int topY = gridRows;
    for (auto& c : p->cells)
        if (c.y < topY) topY = c.y;

    ImGui::Text("Height: %d", gridRows - topY);

    // Actions
    if (ImGui::CollapsingHeader("Actions", 0))
    {
        if (ImGui::Button("Kill Plant")) { p->die(false); ImGui::End(); return; }
        if (ImGui::Button("Kill + Drop Seeds")) { p->die(true); ImGui::End(); return; }
        if (ImGui::Button("Boost Energy")) p->energy += 50.0f;
        if (ImGui::Button("Starve")) p->energy = ENERGY_MIN;
        if (ImGui::Button("Reset Life")) p->lifeSteps = p->maxLifeSteps;
    }

    // Genome
    if (ImGui::CollapsingHeader("Genome", 0))
    {
        if (ImGui::Button("Copy Genome"))
        {
            exampleGenome.clear();
            exampleGenome.reserve(p->genome.size());
            for (int i = 0; i < (int)p->genome.size(); i++)
                exampleGenome[i] = p->genome[i];
            exampleMaxCells = p->maxCells;
        }

        ImGui::SameLine();

        if (ImGui::Button("Paste Genome"))
        {
            if (!exampleGenome.empty())
            {
                p->genome.clear();
                p->genome.reserve(exampleGenome.size());
                for (int i = 0; i < (int)exampleGenome.size(); i++)
                    p->genome.push_back(exampleGenome[i]);
                p->maxCells = exampleMaxCells;
            }
        }
    }

    // Debug
    if (ImGui::CollapsingHeader("Debug", 0))
    {
        ImGui::Checkbox("Freeze Growth", &debugFreezeGrowth);
        ImGui::Checkbox("Infinite Energy", &debugInfiniteEnergy);
        ImGui::Checkbox("Highlight Plant", &debugHighlightPlant);
    }

    ImGui::End();
}


void drawSeedMenu(){
    Seed* s = selectedSeed;
    if (!showEntityMenu || s == nullptr)
        return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(180, 0), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(ImVec2(menuPos.x, menuPos.y), ImGuiCond_Appearing);

    ImGui::Begin(
        "Seed Menu",
        &showEntityMenu,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse
    );

    // Info
    ImGui::Text("Max Cells: %d", s->maxCells);
    ImGui::Text("Genome size: %d", (int)s->genome.size());

    // Actions
    if (ImGui::CollapsingHeader("Actions"))
    {
        if (ImGui::Button("Delete Seed")) { s->die(); ImGui::End(); return; }
    }

    // Genome copy/paste (same as Plant menu)
    if (ImGui::CollapsingHeader("Genome"))
    {
        if (ImGui::Button("Copy Genome"))
        {
            exampleGenome.clear();
            exampleGenome.reserve(s->genome.size());
            for (int i = 0; i < (int)s->genome.size(); i++)
                exampleGenome[i] = s->genome[i];
            exampleMaxCells = s->maxCells;
        }

        ImGui::SameLine();

        if (ImGui::Button("Paste Genome"))
        {
            if (!exampleGenome.empty())
            {
                s->genome.clear();
                s->genome.reserve(exampleGenome.size());
                for (int i = 0; i < (int)exampleGenome.size(); i++)
                    s->genome.push_back(exampleGenome[i]);
                s->maxCells = exampleMaxCells;
            }
        }
    }

    ImGui::End();
}


void drawSettingsMenu(){
    if (!showSettingsMenu) return;

    ImGui::SetNextWindowPos(ImVec2(settingsMenuPos.x, settingsMenuPos.y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(settingsMenuWidth, settingsMenuHeight), ImGuiCond_Once);

    ImGui::Begin("Settings", nullptr);

    // ---------- Collapse toggle via keyboard ----------
    if (requestSettingsCollapseToggle)
    {
        bool collapsed = ImGui::IsWindowCollapsed();
        ImGui::SetWindowCollapsed(!collapsed, ImGuiCond_Always);
        requestSettingsCollapseToggle = false;
    }

    // ---------- Restore Defaults ----------
    if (ImGui::Button("Restore All Defaults"))
    {
        LIFE_STEPS          = 1000;
        UPDATE_TIME         = 0.02f;
        STARTING_ENERGY     = 60.f;
        BASE_CELL_COST      = 2.f;
        SEED_CELL_COST      = 800.f;
        MAX_CELL_MUTATION   = 10;
        MUTATION_RATE       = 0.05f;
        ENERGY_HEIGHT_SCALE = 2.f;
        ENERGY_SOIL_PENALTY = 0.3f;
        ENERGY_ABOVE_BLOCK  = 10;
        ENERGY_MIN          = 0.01f;
        ZOOM_SPEED          = 1.2f;
        MIN_ZOOM            = 0.4f;
        MAX_ZOOM            = 10.f;
    }

    ImGui::Separator();

    // ---------- Helper lambdas ----------
    auto sliderWithDefaultLeft = [](const char* label, int* value, int min, int max, int def)
    {
        ImGui::PushID(label);
        ImGui::BeginGroup();
        if (ImGui::SmallButton("D")) *value = def;
        ImGui::SameLine();
        ImGui::SliderInt(label, value, min, max);
        ImGui::EndGroup();
        ImGui::PopID();
    };

    auto sliderWithDefaultLeftF = [](const char* label, float* value, float min, float max, float def)
    {
        ImGui::PushID(label);
        ImGui::BeginGroup();
        if (ImGui::SmallButton("D")) *value = def;
        ImGui::SameLine();
        ImGui::SliderFloat(label, value, min, max);
        ImGui::EndGroup();
        ImGui::PopID();
    };

    // =================================================
    //                  SIMULATION SETTINGS
    // =================================================
    if (ImGui::CollapsingHeader("Simulation Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Life Time
        sliderWithDefaultLeft("Life steps", &LIFE_STEPS, 0, 5000, 1000);

        // Update Time
        sliderWithDefaultLeftF("Update Time", &UPDATE_TIME, 0.0001f, 0.1f, 0.02f);

        // Other sliders
        sliderWithDefaultLeft("Max Cell Mutation", &MAX_CELL_MUTATION, 0, 50, 10);
        sliderWithDefaultLeftF("Mutation Rate", &MUTATION_RATE, 0.f, 1.f, 0.05f);
    }
    
    // =================================================
    //                  ENERGY SETTINGS
    // =================================================
    if (ImGui::CollapsingHeader("Energy Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        sliderWithDefaultLeftF("Start Energy", &STARTING_ENERGY, 1.f, 500.f, 60.f);
        sliderWithDefaultLeftF("Min Energy", &ENERGY_MIN, 0.f, 1.f, 0.01f);
        sliderWithDefaultLeftF("Normal Cell Cost", &BASE_CELL_COST, 0.f, 50.f, 2.f);
        sliderWithDefaultLeftF("Seed Cell Cost", &SEED_CELL_COST, 1.f, 1000.f, 800.f);
        sliderWithDefaultLeftF("Height Gradient", &ENERGY_HEIGHT_SCALE, 0.f, 10.f, 2.f);
        sliderWithDefaultLeftF("Soil Penalty", &ENERGY_SOIL_PENALTY, 0.f, 1.f, 0.3f);
        sliderWithDefaultLeft("Energy Above Block", &ENERGY_ABOVE_BLOCK, 1, 50, 10);
    }

    // =================================================
    //                  ZOOM SETTINGS
    // =================================================
    if (ImGui::CollapsingHeader("Zoom Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        sliderWithDefaultLeftF("Zoom Speed", &ZOOM_SPEED, 1.0f, 2.f, 1.1f);
        sliderWithDefaultLeftF("Min Zoom", &MIN_ZOOM, 0.1f, 1.f, 0.4f);
        sliderWithDefaultLeftF("Max Zoom", &MAX_ZOOM, 1.f, 20.f, 10.f);
    }

    ImGui::End();
}


// Note: this looks very bad and needs to be modified in the future
void drawSolarOverlay(){
    if (!showSolarOverlay) return;

    for (int y = 0; y < gridRows; y++)
    {
        for (int x = 0; x < gridCols; x++)
        {
            float energy = calculateEnergyAtCell(x, y);

            // ---------- Overlay color ----------
            int alpha = (int)(clamp(energy, 0.f, 1.f) * 255);
            if (alpha > 0)
            {
                DrawRectangle(
                    x * CELL_SIZE * zoom + xOffset,
                    y * CELL_SIZE * zoom + yOffset,
                    CELL_SIZE * zoom - 1,
                    CELL_SIZE * zoom - 1,
                    Color{ 255, 0, 0, (uint8_t)alpha }
                );
            }

            // ---------- Draw tiny energy text ----------
            if (energy > 0.f)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "%.1f", energy);

                float px = x * CELL_SIZE * zoom + xOffset + 1;
                float py = y * CELL_SIZE * zoom + yOffset + 1;

                DrawText(
                    buf,
                    (int)px,
                    (int)py,
                    3,
                    BLACK
                );
            }
        }
    }
}


void drawPlantEnergyOverlay(){
    if (!showPlantEnergyOverlay) return;

    // Position the window
    ImGui::SetNextWindowPos(ImVec2(plantEnergyOverlayPos.x, plantEnergyOverlayPos.y), ImGuiCond_FirstUseEver);
    ImGui::Begin("Plant Energy Overlay", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoCollapse);

    if (plants.empty())
    {
        ImGui::Text("No plants in the simulation.");
    }
    else
    {
        int i = 0;
        for (auto* p : plants)
        {
            i++;

            // Sum energy of the plant’s cells using the global function
            float totalEnergy = 0.f;
            for (auto& c : p->cells)
                totalEnergy += calculateEnergyAtCell(c.x, c.y);

            ImGui::Text("Plant %d: Energy %.2f, Cells %d", i, totalEnergy, (int)p->cells.size());
        }
    }

    ImGui::End();
}


void drawInfo(){
    if (!showInfoWindow) return;

    // ---------- Default position ----------
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

    // Begin window, no X button
    ImGui::Begin("Info", nullptr);

    // ---------- Keyboard Collapse Toggle ----------
    bool isCollapsed = ImGui::IsWindowCollapsed();

    if (requestInfoCollapseToggle)
    {
        ImGui::SetWindowCollapsed(!isCollapsed, ImGuiCond_Always);
        requestInfoCollapseToggle = false;
    }
    // ---------- Contents ----------
    int totalCells = 0;
    for (auto* p : plants) totalCells += (int)p->cells.size();

    ImGui::Text("Plants: %d", (int)plants.size());
    ImGui::Text("Seeds: %d", (int)seeds.size());
    ImGui::Text("Cells: %d", totalCells);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Simulation: %s", simulationPaused ? "Paused" : "Running");
    ImGui::Text("Render: %s", renderEnabled ? "Enabled" : "Disabled");

    ImGui::End();
}


void drawCellProber(){
    if (!showCellProber) return;

    // Set initial position only the first time
    ImGui::SetNextWindowPos(ImVec2(proberPos.x, proberPos.y), ImGuiCond_FirstUseEver);

    // Non-collapsible, always auto-resize, no title bar close button
    ImGui::Begin("Cell Prober", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoCollapse);

    // Set width manually (wider)
    ImGui::SetWindowSize(ImVec2(220, 80), ImGuiCond_Once);

    if (probedCellType == -1)
        ImGui::Text("Click on a cell to probe it.");
    else if (probedCellType == TYPE_SEED)
        ImGui::Text("Cell Type: SEED (%d)", TYPE_SEED);
    else
        ImGui::Text("Cell Type: %d", probedCellType);

    ImGui::End();
}


void drawStartScreen(){
    if (!showStartScreen) return;
    
    #ifdef __linux__ 
    UpdateVideoBackground();
    DrawVideoBackground();
    #endif

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(GetScreenWidth(), GetScreenHeight()), ImGuiCond_Always);

    ImGui::Begin(
        "Welcome to PlantSim!",
        nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    float windowWidth  = ImGui::GetWindowSize().x;
    float windowHeight = ImGui::GetWindowSize().y;

    // =================================================
    //                      TITLE
    // =================================================
    ImGui::Spacing();
    ImGui::SetWindowFontScale(1.8f);

    const char* title = "Welcome to PlantSim!";
    float titleWidth = ImGui::CalcTextSize(title).x;
    ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
    ImGui::TextUnformatted(title);

    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // =================================================
    //                  CONTENT AREA
    // =================================================
    // Reserve space at bottom for the button
    ImGui::BeginChild(
        "StartScreenContent",
        ImVec2(0, windowHeight - 160.0f),
        false
    );

    auto CenteredCollapsingHeader = [&](const char* label) -> bool {
        constexpr float HEADER_WIDTH = 800.0f;

        float startX = (windowWidth - HEADER_WIDTH) * 0.5f;
        ImGui::SetCursorPosX(startX);

        float headerHeight = ImGui::GetFrameHeight();

        ImGui::BeginChild(
            label,
            ImVec2(HEADER_WIDTH, headerHeight),
            false,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);

        bool open = ImGui::CollapsingHeader(
            label,
            ImGuiTreeNodeFlags_Framed
        );

        ImGui::EndChild();
        return open;
    };

    auto textAlignerUnderHeader = [&](const std::vector<const char*>& lines){
        constexpr float HEADER_WIDTH = 400.0f;
        float startX = (windowWidth - HEADER_WIDTH) * 0.5f - 150;
        ImGui::Indent(startX);

        for (auto& line : lines)
            ImGui::Text("%s", line);

        ImGui::Spacing();
        ImGui::Unindent(startX);
    };

    if (CenteredCollapsingHeader("Mouse Controls")){
        textAlignerUnderHeader({
            "Right-click: select plant / seed",
            "Left-click: place seed",
            "Left-click (Prober): inspect cell",
            "Mouse wheel: zoom",
            "Middle mouse drag: pan camera",
        });
    }

    if (CenteredCollapsingHeader("Keyboard Controls")){
        textAlignerUnderHeader({
            "Space: pause / resume simulation",
            "S: toggle settings panel",
            "G: toggle genome panel",
            "I: toggle info panel",
            "P: toggle cell prober",
            "R: toggle rendering",
            "L: place random-genome seed at cursor",
            "F: step one frame",
            "1: toggle solar overlay",
            "2: toggle plant energy overlay",
        });
    }

    if (CenteredCollapsingHeader("Genome")){
        textAlignerUnderHeader({
            "Each plant/seed has a genome: a list of rules controlling growth",
            "Rules determine how cells grow in each direction (left, right, up, down)",
            "Each rule can have conditions, e.g.:",
            "    - Height above/below a value",
            "    - Total number of cells",
            "Seeds can optionally mutate their genome when created",
            "Random genome generator creates new plants/seeds with varied growth patterns",
            "Some cells (like 255) are rarer or special in genome rules"
        });
    }

    if (CenteredCollapsingHeader("Simulation Settings")){
        textAlignerUnderHeader({
            "Life steps: max lifespan of plants",
            "Update time: simulation tick speed",
            "Max Cell Mutation & Mutation Rate: affects genetic variation"
        });
    }

    if (CenteredCollapsingHeader("Energy Settings")){
        textAlignerUnderHeader({
            "Start Energy / Min Energy",
            "Normal Cell Cost / Seed Cell Cost",
            "Height Gradient, Soil Penalty, Energy Above Block"
        });
    }

    if (CenteredCollapsingHeader("Zoom Settings")){
        textAlignerUnderHeader({
            "Zoom speed, min & max zoom levels"
        });
    }
    ImGui::EndChild();


    // =================================================
    //                 CONTINUE BUTTON
    // =================================================
    constexpr float BTN_WIDTH  = 200.0f;
    constexpr float BTN_HEIGHT = 50.0f;
    constexpr float BTN_MARGIN = 40.0f;

    ImGui::SetCursorPos(ImVec2(
        (windowWidth - BTN_WIDTH) * 0.5f,
        windowHeight - BTN_HEIGHT - BTN_MARGIN
    ));

    if (ImGui::Button("Continue", ImVec2(BTN_WIDTH, BTN_HEIGHT)))
    {
        #if defined(PLATFORM_LINUX)
        ShutdownVideoBackground(); 
        #endif
        showStartScreen = false;
    }

    ImGui::End();
}



