#include "raylib.h"
#include "chip8.h"
#include <stdio.h>

void handle_input(chip8_t *cpu) {
    cpu->keypad[0x1] = IsKeyDown(KEY_ONE);
    cpu->keypad[0x2] = IsKeyDown(KEY_TWO);
    cpu->keypad[0x3] = IsKeyDown(KEY_THREE);
    cpu->keypad[0xC] = IsKeyDown(KEY_FOUR);

    cpu->keypad[0x4] = IsKeyDown(KEY_Q);
    cpu->keypad[0x5] = IsKeyDown(KEY_W);
    cpu->keypad[0x6] = IsKeyDown(KEY_E);
    cpu->keypad[0xD] = IsKeyDown(KEY_R);

    cpu->keypad[0x7] = IsKeyDown(KEY_A);
    cpu->keypad[0x8] = IsKeyDown(KEY_S);
    cpu->keypad[0x9] = IsKeyDown(KEY_D);
    cpu->keypad[0xE] = IsKeyDown(KEY_F);

    cpu->keypad[0xA] = IsKeyDown(KEY_Z);
    cpu->keypad[0x0] = IsKeyDown(KEY_X);
    cpu->keypad[0xB] = IsKeyDown(KEY_C);
    cpu->keypad[0xF] = IsKeyDown(KEY_V);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Using: %s <rom_path>\n", argv[0]);
        return -1;
    }

    const int scale = 16;
    const int screenWidth = 64 * scale;
    const int screenHeight = 32 * scale;

    InitWindow(screenWidth, screenHeight, "CHIP-8 Emulator");
    SetTargetFPS(60);

    chip8_t cpu;
    chip8_init(&cpu);

    if (!chip8_load_rom(&cpu, argv[1])) {
        printf("Failed to load ROM: %s\n", argv[1]);
        CloseWindow();
        return -1;
    }

    static uint8_t pixel_decay[64 * 32] = {0};

    while (!WindowShouldClose()) {
        handle_input(&cpu);

        // Run ~10 instructions per 60Hz frame = ~600Hz
        for (int i = 0; i < 10; i++) {
            chip8_cycle(&cpu);
        }

        // Update timers in strictly 60Hz
        if (cpu.delay_timer > 0) cpu.delay_timer--;
        if (cpu.sound_timer > 0) cpu.sound_timer--;

        BeginDrawing();
        ClearBackground(BLACK);

        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                int idx = y * 64 + x;

                if (cpu.display[idx]) {
                    pixel_decay[idx] = 255; // Full brightness when ON
                } else if (pixel_decay[idx] > 0) {
                    // Fade out when OFF
                    pixel_decay[idx] = (pixel_decay[idx] > 40) ? (pixel_decay[idx] - 40) : 0;
                }

                if (pixel_decay[idx] > 0) {
                    const Color color = (Color){ 255, 255, 255, pixel_decay[idx] };
                    DrawRectangle(x * scale, y * scale, scale, scale, color);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}