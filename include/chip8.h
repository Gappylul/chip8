#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32
#define CHIP8_PROGRAM_START 0x200

typedef struct {
    uint8_t memory[CHIP8_MEMORY_SIZE];
    uint8_t V[16];
    uint16_t I;
    uint16_t PC;
    uint16_t stack[16];
    uint8_t SP;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keypad[16];
    uint32_t display[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT];
} chip8_t;

void chip8_init(chip8_t *cpu);

#endif // CHIP8_H
