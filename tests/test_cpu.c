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

void test_alu_add_carry(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0xF0;
    cpu.V[1] = 0x20;

    cpu.memory[0x200] = 0x80;
    cpu.memory[0x201] = 0x14;

    chip8_cycle(&cpu);
    assert(cpu.V[0] == 0x10);
    assert(cpu.V[0xF] == 1);
}

void test_alu_sub_borrow(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0x05;
    cpu.V[1] = 0x10;

    cpu.memory[0x200] = 0x80;
    cpu.memory[0x201] = 0x15;

    chip8_cycle(&cpu);
    assert(cpu.V[0] == (uint8_t)(5 - 16));
    assert(cpu.V[0xF] == 0);
}

void test_alu_shifts(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0b10000001;

    cpu.memory[0x200] = 0x80;
    cpu.memory[0x201] = 0x06;
    chip8_cycle(&cpu);
    assert(cpu.V[0] == 0b01000000);
    assert(cpu.V[0xF] == 1);

    cpu.V[0] = 0b10000001;

    cpu.memory[0x202] = 0x80;
    cpu.memory[0x203] = 0x0E;
    chip8_cycle(&cpu);
    assert(cpu.V[0] == 0b00000010);
    assert(cpu.V[0xF] == 1);
}

void test_opcode_9xy0(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0x11;
    cpu.V[1] = 0x22;

    // 9010 -> Skip if V0 != V1 (Should skip)
    cpu.memory[0x200] = 0x90;
    cpu.memory[0x201] = 0x10;
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x204);

    // 9010 -> Skip if V0 != V1 (Should not skip)
    cpu.V[1] = 0x11;
    cpu.PC = 0x202;
    cpu.memory[0x202] = 0x90;
    cpu.memory[0x203] = 0x10;
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x204);
}

void test_opcode_annn(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    // A123 -> I = 0x123
    cpu.memory[0x200] = 0xA1;
    cpu.memory[0x201] = 0x23;
    chip8_cycle(&cpu);
    assert(cpu.I == 0x0123);
}

void test_opcode_bnnn(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0x42;
    // B100 -> PC = 0x100 + V0 = 0x142
    cpu.memory[0x200] = 0xB1;
    cpu.memory[0x201] = 0x00;
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x0142);
}

void test_opcode_cxkk(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    // C00F -> V0 = rand() & 0x0F
    cpu.memory[0x200] = 0xC0;
    cpu.memory[0x201] = 0x0F;
    chip8_cycle(&cpu);

    assert((cpu.V[0] & 0xF0) == 0x00);
}

int main(void) {
    test_initialization();
    test_opcodes_6xkk_7xkk();
    test_call_and_return();
    test_alu_add_carry();
    test_alu_sub_borrow();
    test_alu_shifts();
    test_opcode_9xy0();
    test_opcode_annn();
    test_opcode_bnnn();
    test_opcode_cxkk();

    printf("OK: All tests passed.\n");
    return 0;
}