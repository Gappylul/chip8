#include <stdio.h>
#include <assert.h>
#include "chip8.h"

int main(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    assert(cpu.PC == CHIP8_PROGRAM_START && "Program Counter should start at 0x200");
    assert(cpu.V[0] == 0 && "Registers should be zeroed");
    assert(cpu.I == 0 && "Index register should be zeroed");
    assert(cpu.SP == 0 && "Stack pointer should be zeroed");
    assert(cpu.memory[0] == 0 && "Memory should be zeroed");

    printf("OK: CPU initialized successfully.\n");
    return 0;
}