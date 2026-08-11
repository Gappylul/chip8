#include "raylib.h"
#include "chip8.h"

int main(void) {
    const int screenWidth = 1024;
    const int screenHeight = 512;

    InitWindow(screenWidth, screenHeight, "CHIP-8 Emulator");
    SetTargetFPS(60);

    chip8_t cpu;
    chip8_init(&cpu);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("CHIP-8 Core Initialized", 10, 10, 20, DARKGREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}