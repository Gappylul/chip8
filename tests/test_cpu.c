#include <stdio.h>
#include <assert.h>
#include "chip8.h"

void test_initialization(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    assert(cpu.PC == CHIP8_PROGRAM_START);
    assert(cpu.memory[CHIP8_FONTSET_START] == 0xF0);
}

void test_opcodes_6xkk_7xkk(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.memory[0x200] = 0x61;
    cpu.memory[0x201] = 0x25;
    chip8_cycle(&cpu);
    assert(cpu.V[1] == 0x25);

    cpu.memory[0x202] = 0x71;
    cpu.memory[0x203] = 0x05;
    chip8_cycle(&cpu);
    assert(cpu.V[1] == 0x2A);
}

void test_call_and_return(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.memory[0x200] = 0x24;
    cpu.memory[0x201] = 0x00;

    cpu.memory[0x400] = 0x00;
    cpu.memory[0x401] = 0xEE;

    chip8_cycle(&cpu);
    assert(cpu.PC == 0x400);
    assert(cpu.SP == 1);
    assert(cpu.stack[0] == 0x202);

    chip8_cycle(&cpu);
    assert(cpu.PC == 0x202);
    assert(cpu.SP == 0);
}

int main(void) {
    test_initialization();
    test_opcodes_6xkk_7xkk();
    test_call_and_return();

    printf("OK: All tests passed.\n");
    return 0;
}