
// ---------------------------------------------------------------------------
// I basically stole this idea from casey, and tried implementing it myself 
// after looking at his code. 
// I felt that I could learn from it so I went and did that
// ---------------------------------------------------------------------------

#ifndef INSTRUCTION
#define INSTRUCTION(mnemonic, ...) {Op_##mnemonic, __VA_ARGS__},
#endif
#ifndef INSTRUCTION_ALT
#define INSTRUCTION_ALT INSTRUCTION
#endif

#ifndef INSTRUCTION_EMPTY
#define INSTRUCTION_EMPTY(...) 
#endif

/* #define Op(Value) {T_op, sizeof(#Value) - 1,0b##Value} */
#define B(Value) {T_byte, sizeof(#Value) - 1, 0b##Value}

#define W {T_w, 1}
#define D {T_d, 1}
#define S {T_s, 1}

#define MOD {T_mod, 2}
#define REG {T_reg, 3}
#define RM {T_rm, 3}
#define SR {T_sr, 2}

#define DATA {T_data, 8}
#define DATA_IF_W {T_data_if_w, 0}

#define ADDR {T_addr, 8}, {T_addr_w, 8}

#define PRINT_LEN {T_print_len, 0}

#define ARITH_CODE {T_arith_code, 3}

#define IMP_D(Value) {T_imp_d, 0, Value}
#define IMP_PRINT_LEN(Value) {T_imp_print_len, 0, Value}

#define IMP_REG(Value) {T_imp_reg, 0, Value}
#define IMP_IMM {T_imp_imm, 0, 1}

#define IMP_MOD(Value) {T_imp_mod, 0, Value}
#define IMP_RM(Value) {T_imp_rm, 0, Value}

#define IMP_JMP {T_imp_jmp, 0, 1}

#define IMP_DIR_MEM IMP_MOD(00), IMP_RM(0b110)

// MOVS
INSTRUCTION(mov, {B(100010), D, W, MOD, REG, RM})
INSTRUCTION_ALT(mov, {B(1100011), W, MOD, B(000), IMP_IMM, RM, DATA, DATA_IF_W, IMP_D(0), IMP_PRINT_LEN(1)})
INSTRUCTION_ALT(mov, {B(1011), W, REG, IMP_IMM, DATA, DATA_IF_W, IMP_D(1)})

INSTRUCTION_ALT(mov, {B(1010000), W, IMP_REG(Register_a), IMP_DIR_MEM, IMP_D(1)})
INSTRUCTION_ALT(mov, {B(1010001), W, IMP_REG(Register_a), IMP_DIR_MEM, IMP_D(0)})

INSTRUCTION_ALT(mov, {B(100011), D, B(0), MOD, B(0), SR, RM})
// END MOVS


// ARITH 
INSTRUCTION(arith, {B(100000), S, W, MOD, ARITH_CODE, IMP_IMM, RM, DATA, DATA_IF_W, IMP_D(0), PRINT_LEN})

// ARITH CODES
    INSTRUCTION_EMPTY(add)
    INSTRUCTION_EMPTY(or)
    INSTRUCTION_EMPTY(adc)
    INSTRUCTION_EMPTY(sbb)
    INSTRUCTION_EMPTY(and)
    INSTRUCTION_EMPTY(sub)
    INSTRUCTION_EMPTY(xor)
    INSTRUCTION_EMPTY(cmp)
// END ARITH CODES

// END ARITH

// ADD 
INSTRUCTION_ALT(add, {B(000000), D, W, MOD, REG, RM})
INSTRUCTION_ALT(add, {B(0000010), W, IMP_REG(Register_a), IMP_IMM, DATA, DATA_IF_W, IMP_D(1)})
// END ADD

// SUB 
INSTRUCTION_ALT(sub, {B(001010), D, W, MOD, REG, RM})
INSTRUCTION_ALT(sub, {B(0010110), W, IMP_REG(Register_a), IMP_IMM, DATA, DATA_IF_W, IMP_D(1)})
// END SUB

// CMP 
INSTRUCTION_ALT(cmp, {B(001110), D, W, MOD, REG, RM})
INSTRUCTION_ALT(cmp, {B(0011110), W, IMP_REG(Register_a), IMP_IMM, DATA, DATA_IF_W, IMP_D(1)})
// END CMP

// JMP
INSTRUCTION(je,     {B(01110100), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jl,     {B(01111100), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jle,    {B(01111110), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jb,     {B(01110010), IMP_JMP, DATA, IMP_D(1)})

INSTRUCTION(jbe,    {B(01110110), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jp,     {B(01111010), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jo,     {B(01110000), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(js,     {B(01111000), IMP_JMP, DATA, IMP_D(1)})

INSTRUCTION(jne,    {B(01110101), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jnl,    {B(01111101), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jnle,   {B(01111111), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jnb,    {B(01110011), IMP_JMP, DATA, IMP_D(1)})

INSTRUCTION(jnbe,   {B(01110111), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jnp,    {B(01111011), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jno,    {B(01110001), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jns,    {B(01111001), IMP_JMP, DATA, IMP_D(1)})

INSTRUCTION(loop,   {B(11100010), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(loopz,  {B(11100001), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(loopnz, {B(11100000), IMP_JMP, DATA, IMP_D(1)})
INSTRUCTION(jcxz,   {B(11100011), IMP_JMP, DATA, IMP_D(1)})
// END JMP


#undef INSTRUCTION
#undef INSTRUCTION_ALT
#undef INSTRUCTION_EMPTY
