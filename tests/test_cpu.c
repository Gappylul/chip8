#include <stdio.h>
#include <assert.h>
#include <string.h>

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

void test_opcode_f_timers(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[1] = 0x30;

    // F115 -> Set delay_timer = V1
    cpu.memory[0x200] = 0xF1;
    cpu.memory[0x201] = 0x15;
    chip8_cycle(&cpu);
    assert(cpu.delay_timer == 0x30);

    // F207 -> Set V2 = delay_timer
    cpu.memory[0x202] = 0xF2;
    cpu.memory[0x203] = 0x07;
    chip8_cycle(&cpu);
    assert(cpu.V[2] == 0x30);
}

void test_opcode_fx29_font_sprite(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    // Digit 'A' (0x0A) -> Each font sprite is 5 bytes tall
    cpu.V[0] = 0x0A;

    // F029 -> Set I to location of sprite for digit in V0
    cpu.memory[0x200] = 0xF0;
    cpu.memory[0x201] = 0x29;
    chip8_cycle(&cpu);

    assert(cpu.I == CHIP8_FONTSET_START + (0x0A * 5));
    assert(cpu.memory[cpu.I] == 0xF0); // First byte of character 'A' sprite
}

void test_opcode_fx33_bcd(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[2] = 254; // Hundreds: 2, Tens: 5, Ones: 4
    cpu.I = 0x300;

    // F233 -> Store BCD of V2 at memory[I..I+2]
    cpu.memory[0x200] = 0xF2;
    cpu.memory[0x201] = 0x33;
    chip8_cycle(&cpu);

    assert(cpu.memory[0x300] == 2);
    assert(cpu.memory[0x301] == 5);
    assert(cpu.memory[0x302] == 4);
}

void test_opcode_fx55_fx65_reg_dump(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0x11;
    cpu.V[1] = 0x22;
    cpu.V[2] = 0x33;
    cpu.I = 0x400;

    // F255 -> Dump V0..V2 to memory[I..I+2]
    cpu.memory[0x200] = 0xF2;
    cpu.memory[0x201] = 0x55;
    chip8_cycle(&cpu);

    assert(cpu.memory[0x400] == 0x11);
    assert(cpu.memory[0x401] == 0x22);
    assert(cpu.memory[0x402] == 0x33);

    // Clear registers and read back from memory using F265
    cpu.V[0] = 0; cpu.V[1] = 0; cpu.V[2] = 0;
    cpu.memory[0x202] = 0xF2;
    cpu.memory[0x203] = 0x65;
    chip8_cycle(&cpu);

    assert(cpu.V[0] == 0x11);
    assert(cpu.V[1] == 0x22);
    assert(cpu.V[2] == 0x33);
}

void test_opcode_input(void) {
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0] = 0x05; // We are checking key 5
    cpu.keypad[0x05] = 1; // Simulate key 5 being pressed

    // EX9E -> Skip if key in V0 is pressed (Should skip)
    cpu.memory[0x200] = 0xE0;
    cpu.memory[0x201] = 0x9E;
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x204);

    // EXA1 -> Skip if key in V0 is NOT pressed (Should NOT skip)
    cpu.PC = 0x204;
    cpu.memory[0x204] = 0xE0;
    cpu.memory[0x205] = 0xA1;
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x206); // Only advanced 2 bytes

    // FX0A -> Wait for key press
    cpu.PC = 0x206;
    cpu.memory[0x206] = 0xF1; // Store in V1
    cpu.memory[0x207] = 0x0A;

    // First, try with no keys pressed
    memset(cpu.keypad, 0, sizeof(cpu.keypad));
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x206); // PC should not advance (waiting)

    // Now press a key (key 0x0A)
    cpu.keypad[0x0A] = 1;
    chip8_cycle(&cpu);
    assert(cpu.PC == 0x208); // PC advanced!
    assert(cpu.V[1] == 0x0A); // V1 holds the pressed key
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
    test_opcode_f_timers();
    test_opcode_fx29_font_sprite();
    test_opcode_fx33_bcd();
    test_opcode_fx55_fx65_reg_dump();
    test_opcode_input();

    printf("OK: All tests passed.\n");
    return 0;
}