#include <stdio.h>

#include "chip8.h"
#include <string.h>
#include <stdlib.h>

const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(chip8_t *cpu) {
    memset(cpu, 0, sizeof(chip8_t));
    cpu->PC = CHIP8_PROGRAM_START;

    for (int i = 0; i < 80; i++) {
        cpu->memory[CHIP8_FONTSET_START + i] = fontset[i];
    }
}

bool chip8_load_rom(chip8_t *cpu, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    const long size = ftell(file);
    rewind(file);

    const long max_size = CHIP8_MEMORY_SIZE - CHIP8_PROGRAM_START;
    if (size > max_size) {
        fclose(file);
        return false;
    }

    fread(&cpu->memory[CHIP8_PROGRAM_START], 1, size, file);
    fclose(file);
    return true;
}

void chip8_cycle(chip8_t *cpu) {
    const uint16_t opcode = (cpu->memory[cpu->PC] << 8) | cpu->memory[cpu->PC + 1];
    cpu->PC += 2;

    const uint8_t x = (opcode & 0x0F00) >> 8;
    const uint8_t y = (opcode & 0x00F0) >> 4;
    const uint8_t kk = opcode & 0x00FF;
    const uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0: // 00E0: Clear screen
                    memset(cpu->display, 0, sizeof(cpu->display));
                    break;
                case 0x00EE: // 00EE: Return from subroutine
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                    break;
                default:
                    printf("Unknown opcode [0x0000]: 0x%04X\n", opcode);
                    break;
            }
            break;

        case 0x1000: // 1NNN: Jump to address NNN
            cpu->PC = nnn;
            break;

        case 0x2000: // 2NNN: Call subroutine at NNN
            cpu->stack[cpu->SP] = cpu->PC;
            cpu->SP++;
            cpu->PC = nnn;
            break;

        case 0x3000: // 3XKK: Skip next instruction if Vx = KK
            if (cpu->V[x] == kk) {
                cpu->PC += 2;
            }
            break;

        case 0x4000: // 4XKK: Skip next instruction if Vx != KK
            if (cpu->V[x] != kk) {
                cpu->PC += 2;
            }
            break;

        case 0x5000: // 5XY0: Skip next instruction if Vx == Vy
            if (opcode & 0x000F) break; // Must end in 0
            if (cpu->V[x] == cpu->V[y]) {
                cpu->PC += 2;
            }
            break;

        case 0x6000: // 6XKK: Set Vx = KK
            cpu->V[x] = kk;
            break;

        case 0x7000: // 7XKK: Set Vx += KK
            cpu->V[x] += kk;
            break;

        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0: // 8XY0: Set Vx = Vy
                    cpu->V[x] = cpu->V[y];
                    break;

                case 0x1: // 8XY1: Set Vx = Vx | Vy
                    cpu->V[x] |= cpu->V[y];
                    break;

                case 0x2: // 8XY2: Set Vx = Vx & Vy
                    cpu->V[x] &= cpu->V[y];
                    break;

                case 0x3: // 8XY3: Set Vx = Vx ^ Vy
                    cpu->V[x] ^= cpu->V[y];
                    break;

                case 0x4: {
                    // 8XY4: Set Vx += Vy, set VF = carry
                    const uint16_t sum = cpu->V[x] + cpu->V[y];
                    cpu->V[x] = sum & 0xFF;
                    cpu->V[0xF] = (sum > 255) ? 1 : 0;
                    break;
                }

                case 0x0005: { // 8XY5: Set Vx = Vx - Vy, set VF = NOT borrow
                    const uint8_t not_borrow = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
                    cpu->V[x] = cpu->V[x] - cpu->V[y];
                    cpu->V[0xF] = not_borrow;
                    break;
                }

                case 0x0006: { // 8XY6: Set Vx = Vx >> 1, set VF = LSB
                    const uint8_t lsb = cpu->V[x] & 0x01;
                    cpu->V[x] >>= 1;
                    cpu->V[0xF] = lsb;
                    break;
                }

                case 0x0007: { // 8XY7: Set Vx = Vy - Vx, set VF = NOT borrow
                    const uint8_t not_borrow = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
                    cpu->V[x] = cpu->V[y] - cpu->V[x];
                    cpu->V[0xF] = not_borrow;
                    break;
                }

                case 0x000E: { // 8XYE: Set Vx = Vx << 1, set VF = MSB
                    const uint8_t msb = (cpu->V[x] & 0x80) >> 7;
                    cpu->V[x] <<= 1;
                    cpu->V[0xF] = msb;
                    break;
                }

                default:
                    printf("Unknown opcode [0x8000]: 0x%04X\n", opcode);
                    break;
            }
            break;

        case 0x9000: // 9XY0: Skip next instruction if Vx != Vy
            if ((opcode & 0x000F) == 0) {
                if (cpu->V[x] != cpu->V[y]) {
                    cpu->PC += 2;
                }
            }
            break;

        case 0xA000: // ANNN: Set index register I = NNN
            cpu->I = nnn;
            break;

        case 0xB000: // BNNN: Jump to location NNN + V0
            cpu->PC = nnn + cpu->V[0];
            break;

        case 0xC000: // CXKK: Set Vx = random byte & KK
            cpu->V[x] = (rand() % 256) & kk;
            break;

        default:
            printf("Unimplemented opcode: 0x%04X\n", opcode);
            break;
    }
}