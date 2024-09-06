#pragma once

typedef enum : uint16_t {
    CF = 1 << 0,
    PF = 1 << 2,
    AF = 1 << 4,
    ZF = 1 << 6,
    SF = 1 << 7,
    TF = 1 << 8,
    IF = 1 << 9,
    DF = 1 << 10,
    OF = 1 << 11,
} Flags;

typedef struct state_8086 {
    int16_t registers[12];

    uint16_t flag_reg;
    uint16_t instruction_pointer;

    memory main_memory;
} state_8086;

void run_instruction(FILE *out, instruction inst, state_8086 *state,
        bool modified_regs[Register_count]);
