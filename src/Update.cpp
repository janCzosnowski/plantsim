#include "Helpers.h"
#include "Globals.h"
#include "Plant.h"
#include "Seed.h"

void updateSeeds() {
    for (int i = (int)seeds.size() - 1; i >= 0; i--) {
        Seed* s = seeds[i];

        // Move one cell per update
        int belowY = s->y + 1;

        if (belowY >= gridRows) {
            plants.push_back(new Plant(s->x, s->y, s->genome, s->maxCells));
            s->die();             // mark dead, remove from vector
            delete s;             // free memory
            continue;
        }

        if (occupied[s->x][belowY] && isPlantCell(s->x, belowY)) {
            s->die();
            delete s;
            continue;
        }

        // Move seed down
        occupied[s->x][s->y] = false;
        s->y = belowY;
        occupied[s->x][s->y] = true;
    }
}


void updatePlants()
{
    // iterate backwards so erase is safe
    for (int i = (int)plants.size() - 1; i >= 0; i--)
    {
        Plant* p = plants[i];

        if (p->alive)
        {
            p->update();
        }

        if (!p->alive)
        {
            delete p;                // free memory
            plants.erase(plants.begin() + i);  // remove from vector
        }
    }
}


void updateSimulation(float dt, double &simTimer)
{
    bool shouldUpdate = !simulationPaused || stepOnce;

    if (!shouldUpdate) return;

    simTimer += dt;

    while (simTimer >= UPDATE_TIME)
    {
        simTimer -= UPDATE_TIME;

        updateSeeds();
        updatePlants();

        if (stepOnce)
        {
            stepOnce = false;
            break;
        }
    }
}
