#include <stdio.h>
#include <assert.h>
#include "chip8.h"

void test_initialization() {
    chip8_t cpu;
    chip8_init(&cpu);

    assert(cpu.PC == CHIP8_PROGRAM_START);
    assert(cpu.memory[CHIP8_FONTSET_START] == 0xF0);
}

void test_fetch_cycle() {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.memory[0x200] = 0xA2;
    cpu.memory[0x201] = 0xF0;

    chip8_cycle(&cpu);

    assert(cpu.PC == CHIP8_PROGRAM_START + 2);
}

int main(void) {
    test_initialization();
    test_fetch_cycle();

    printf("OK: All tests passed.\n");
    return 0;
}