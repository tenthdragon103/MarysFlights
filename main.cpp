#include "raylib.h"

#include "simulation/Simulation.h"
#include "graphics/Renderer.h"

int main()
{
    InitWindow(
        1280,
        720,
        "MarysFlights"
    );

    SetTargetFPS(60);

    Simulation simulation;
    Renderer renderer;

    simulation.startNew();

    while (!WindowShouldClose())
    {
        const double deltaTime =
            GetFrameTime();

        simulation.update(
            deltaTime
        );

        renderer.updateCamera(
            simulation
        );
        renderer.updateSelection(
            simulation.getSolarSystem()
        );

        if (IsKeyPressed(KEY_TAB))
        {
            renderer.focusNextBody(
                simulation.getSolarSystem()
            );
        }

        BeginDrawing();

        ClearBackground(BLACK);

        renderer.draw(
            simulation
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}