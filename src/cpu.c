#include "chip8.h"
#include <string.h>

void chip8_init(chip8_t *cpu) {
    memset(cpu, 0, sizeof(chip8_t));
    cpu->PC = CHIP8_PROGRAM_START;
}