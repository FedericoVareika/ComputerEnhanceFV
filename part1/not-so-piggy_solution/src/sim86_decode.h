#pragma once

#define MEM_SIZE_EXP 20

#define ArraySize(arr) (sizeof(arr) / sizeof(*arr))
typedef enum : uint8_t { false, true } bool;

typedef enum instruction_type : uint8_t {
    T_end = 0,

    T_byte,

    T_d,
    T_w,
    T_s,

    T_mod,
    T_reg,
    T_sr,
    T_rm,

    T_data,
    T_data_if_w,

    T_print_len,
    T_arith_code,

    T_imp_d,
    T_imp_w,
    T_imp_print_len,

    T_imp_mod,
    T_imp_reg,
    T_imp_rm,
    T_imp_imm,

    T_imp_jmp,
} instruction_type;

typedef struct instruction_bits {
    instruction_type type;
    uint8_t size;
    uint8_t val;
} instruction_bits;

typedef enum : uint32_t {
    Op_none,

#define INSTRUCTION(Mnemonic, ...) Op_##Mnemonic,
#define INSTRUCTION_EMPTY(Mnemonic) Op_##Mnemonic,
#define INSTRUCTION_ALT(...)
#include "sim86_table.inl"

} operation_type;

typedef struct instruction_format {
    operation_type Op;
    instruction_bits Bits[16];
} instruction_format;

typedef enum : uint8_t {
    d_flag = 0b1,
    w_flag = 0b10,
    s_flag = 0b100,

    print_len_flag = 0b1000,
    is_arith_flag = 0b10000,
    is_jmp_flag = 0b100000,
} flag_enum;

typedef enum : uint8_t {
    Register_a,
    Register_b,
    Register_c,
    Register_d,

    Register_sp,
    Register_bp,
    Register_di,
    Register_si,

    Register_cs,
    Register_ds,
    Register_ss,
    Register_es,

    Register_count,
} register_enum;

typedef enum : uint8_t {
    Lower_half,
    Higher_half,
    None,
} register_half_enum;

typedef struct register_ {
    register_enum val;
    register_half_enum half;
} register_;

typedef enum : uint8_t {
    eff_add_bx_si,
    eff_add_bx_di,
    eff_add_bp_si,
    eff_add_bp_di,
    eff_add_si,
    eff_add_di,
    eff_add_bp,
    eff_add_bx,

    eff_add_direct,
} effective_address_enum;

typedef enum : uint8_t {
    Operand_none,
    Operand_memory,
    Operand_memory_8,
    Operand_memory_16,
    Operand_register,
    Operand_immediate,
} operand_type;

typedef struct operand {
    operand_type operand_type;
    union {
        register_ reg;
        int16_t immediate;
        struct {
            effective_address_enum eff_addr;
            int16_t disp;
        } memory;
    };
} operand;

typedef struct instruction {
    operation_type operation;

    uint8_t operand_count;
    operand operands[2];

    uint8_t flags;
} instruction;


instruction parse_instruction(mem_access *at);
