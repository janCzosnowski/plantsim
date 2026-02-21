#include "Helpers.h"
#include "Globals.h"
#include "Plant.h"
#include "Seed.h"



// --- Convert single DirectionRule to string ---
std::string directionToString(const DirectionRule& d, bool addSpace) {
    std::string s = std::to_string(d.ifCellType) + "," +
                    std::to_string(d.elseCellType) + "," +
                    std::to_string(d.condition.type) + "," +
                    std::to_string(d.condition.value) + ";";
    if(addSpace) s += " ";
    return s;
}

// --- Convert ConditionalGenomeRule to string ---
std::string ruleToString(const ConditionalGenomeRule& r, bool addSpace) {
    return directionToString(r.left, addSpace) +
           directionToString(r.right, addSpace) +
           directionToString(r.up, addSpace) +
           directionToString(r.down, addSpace);
}

// --- Convert entire genome map to string ---
std::string genomeMapToString(const std::unordered_map<int, ConditionalGenomeRule>& genome, bool addSpace) {
    std::string out;
    for (auto& [label, rule] : genome) {
        out += std::to_string(label) + ": {" + ruleToString(rule, addSpace) + "}\n";
    }
    return out;
}

// --- Parse a single line {…} into ConditionalGenomeRule ---
ConditionalGenomeRule parseGenomeLine(const std::string& line) {
    size_t start = 0, end;
    std::vector<DirectionRule> dirs;
    for(int i=0;i<4;i++){
        end = line.find(';', start);
        std::string part = (end == std::string::npos) ? line.substr(start) : line.substr(start, end-start);
        part = trim(part);
        if(part.empty()) part = "0,0,0,0"; // safety

        int ifCell=0, elseCell=0, condType=0, condVal=0;
        sscanf(part.c_str(), "%d,%d,%d,%d", &ifCell, &elseCell, &condType, &condVal);
        dirs.push_back(DirectionRule((uint8_t)ifCell, (uint8_t)elseCell, ConditionalValue((ConditionalType)condType, condVal)));

        if(end == std::string::npos) break;
        start = end+1;
    }
    return ConditionalGenomeRule(dirs[0], dirs[1], dirs[2], dirs[3]);
}

// --- Parse a labeled line "17: {…}" into pair<int,ConditionalGenomeRule> ---
std::pair<int, ConditionalGenomeRule> parseLabeledGenomeLine(const std::string& line) {
    size_t colonPos = line.find(':');
    if(colonPos == std::string::npos) throw std::runtime_error("Invalid genome line: missing ':'");
    int label = std::stoi(trim(line.substr(0, colonPos)));

    size_t braceStart = line.find("{", colonPos);
    size_t braceEnd   = line.find("}", colonPos);
    if(braceStart == std::string::npos || braceEnd == std::string::npos) throw std::runtime_error("Invalid genome line: missing braces");

    std::string inner = line.substr(braceStart+1, braceEnd-braceStart-1);
    return {label, parseGenomeLine(inner)};
}

// --- Parse entire labeled genome text ---
std::unordered_map<int, ConditionalGenomeRule> parseLabeledGenomeText(const std::string& text) {
    std::unordered_map<int, ConditionalGenomeRule> out;
    std::stringstream ss(text);
    std::string line;
    int lineNumber = 0;

    while (std::getline(ss, line)) {
        lineNumber++;
        line = trim(line);
        if (line.empty()) continue;

        try {
            auto p = parseLabeledGenomeLine(line);
            out[p.first] = p.second;
        } catch (const std::exception& e) {
            // throw with line number
            throw std::runtime_error("Parse error at line " + std::to_string(lineNumber));
        }
    }
    return out;
}

float calculateEnergyAtCell(int x, int y)
{
    // Clamp coordinates
    x = clamp(x, 0, gridCols - 1);
    y = clamp(y, 0, gridRowsFull - 1);

    // ---------- Block check (cells above) ----------
    for (int dy = 1; dy <= ENERGY_ABOVE_BLOCK; dy++)
    {
        int yCheck = y - dy;
        if (yCheck < 0) break;
        if (occupied[x][yCheck]) return 0.f; // blocked
    }

    // ---------- Height-based energy from soil ----------
    int soilTop = gridRowsFull - SOIL_LEVEL;          // top row of soil
    int distanceAboveSoil = soilTop - y;             // 0 at soil, increases upward
    distanceAboveSoil = std::max(distanceAboveSoil, 0);

    float baseEnergy = ENERGY_MIN; // give soil some energy
    float energy = baseEnergy + (distanceAboveSoil * ENERGY_HEIGHT_SCALE) / 500.f;

    // ---------- Soil penalty ----------
    if (y >= gridRowsFull - SOIL_LEVEL)              // inside soil layer
        energy *= ENERGY_SOIL_PENALTY;

    return energy;
}

bool isPlantCell(int x,int y){ for(auto* p:plants) for(auto& c:p->cells) if(c.x==x && c.y==y) return true; return false; }

Vector2 ScreenToWorld(int sx,int sy){ return {(sx-xOffset)/(CELL_SIZE*zoom),(sy-yOffset)/(CELL_SIZE*zoom)}; }

std::string getAssetPath(const std::string& relativePath) {
    const char* appDir = getenv("APPDIR");  // set automatically when running AppImage
    if(appDir)
        return std::string(appDir) + "/usr/share/PlantSim/assets/" + relativePath;
    else
        return "assets/" + relativePath;  // fallback for running outside AppImage
}
