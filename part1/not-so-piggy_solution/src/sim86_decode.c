
instruction_format instruction_lookup[] = {
#include "sim86_table.inl"
};

char *operation_lookup[] = {
    "",
#define INSTRUCTION(mnemonic, ...) #mnemonic,
#define INSTRUCTION_EMPTY(mnemonic, ...) #mnemonic,
#define INSTRUCTION_ALT(...)
#include "sim86_table.inl"
};

register_enum register_lookup[][2] = {
    {Register_a, Register_a},  {Register_c, Register_c},
    {Register_d, Register_d},  {Register_b, Register_b},
    {Register_a, Register_sp}, {Register_c, Register_bp},
    {Register_d, Register_si}, {Register_b, Register_di},

    {Register_es, Register_es}, {Register_cs, Register_cs},
    {Register_ss, Register_ss}, {Register_ds, Register_ds},
};

char *register_str_lookup[][3] = {
    {"al", "ah", "ax"}, {"bl", "bh", "bx"},
    {"cl", "ch", "cx"}, {"dl", "dh", "dx"},

    {"sp", "sp", "sp"}, {"bp", "bp", "bp"},
    {"di", "di", "di"}, {"si", "si", "si"},

    {"cs", "cs", "cs"}, {"ds", "ds", "ds"},
    {"ss", "ss", "ss"}, {"es", "es", "es"},
};

char *memory_eff_addr_str_lookup[] = {
    "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx",

    "", // Direct eff addr
};

static int lookup_instruction(uint8_t byte) {
    for (int lookup_idx = 0; lookup_idx < ArraySize(instruction_lookup);
         lookup_idx++) {
        instruction_format format = instruction_lookup[lookup_idx];
        instruction_bits code = format.Bits[0];
        int byte_offset = 8 - code.size;
        if (code.type == T_byte && code.val == byte >> byte_offset)
            return lookup_idx;
    }
    return -1;
}

static uint8_t get_bits(uint8_t byte, uint8_t size, uint8_t offset) {
    // (0b12345678, 3, 2) ==> 0b345
    return (uint8_t)(byte << offset) >> (sizeof(uint8_t) * 8 - size);
}

static void set_reg(uint8_t reg, uint8_t idx, instruction *inst) {
    uint8_t w = inst->flags & w_flag ? 1 : 0;
    inst->operands[idx].reg.val = register_lookup[reg][w];
    if (w)
        inst->operands[idx].reg.half = None;
    else
        inst->operands[idx].reg.half = reg >> 2;
}

static void decode_identifier(instruction *inst, instruction_bits *identifier,
                              uint8_t current_byte, uint8_t bit_offset,
                              bool *check_disp, uint8_t *operand_idx,
                              mem_access *at) {
    uint8_t id_value = get_bits(current_byte, identifier->size, bit_offset);
    switch (identifier->type) {

    case T_end:
        break;

    case T_byte: {
        assert(id_value == identifier->val);
    } break;

    case T_imp_d:
        id_value = identifier->val;
    case T_d: {
        if (id_value)
            inst->flags |= d_flag;
    } break;

    case T_imp_w:
        id_value = identifier->val;
    case T_w: {
        if (id_value)
            inst->flags |= w_flag;
    } break;

    case T_s: {
        if (id_value)
            inst->flags |= s_flag;
    } break;

    case T_imp_mod:
        id_value = identifier->val;
    case T_mod: {
        inst->operands[1].operand_type = id_value + 1;
        if (id_value + 1 == Operand_memory_8 ||
            id_value + 1 == Operand_memory_16) {
            *check_disp = true;
        }
    } break;

    case T_imp_reg:
        id_value = identifier->val;
    case T_reg: {
        uint8_t idx = (*operand_idx)++;
        inst->operands[idx].operand_type = Operand_register;
        set_reg(id_value, idx, inst);
    } break;

    case T_imp_rm:
        id_value = identifier->val;
    case T_rm: {
        uint8_t idx = (*operand_idx)++;
        if (inst->operands[idx].operand_type == Operand_memory &&
            id_value == 0b110) {
            inst->operands[idx].memory.eff_addr = eff_add_direct;
            *check_disp = true;
        } else if (inst->operands[idx].operand_type == Operand_register) {
            set_reg(id_value, idx, inst);
        } else {
            inst->operands[idx].memory.eff_addr = id_value;
        }
    } break;

    case T_sr: {
        inst->flags |= w_flag;
        uint8_t idx = (*operand_idx)++;
        inst->operands[idx].operand_type = Operand_register;
        set_reg(id_value + Register_cs, idx, inst);
    } break;

    case T_data: {
        uint8_t idx = 0;
        if (inst->operands[idx].operand_type != Operand_immediate)
            idx = 1;
        inst->operands[idx].immediate = (int8_t)id_value;
    } break;

    case T_data_if_w: {
        if (inst->flags & w_flag && !(inst->flags & s_flag)) {
            uint8_t idx = 0;
            if (inst->operands[idx].operand_type != Operand_immediate)
                idx = 1;
            id_value = current_byte;
            int16_t imm = inst->operands[idx].immediate;
            imm = (imm & 0xff) | (id_value << 8);
            inst->operands[idx].immediate = imm;
            identifier->size = 8;
        }
    } break;

    case T_imp_imm: {
        inst->operands[(*operand_idx)++].operand_type = Operand_immediate;
    } break;

    case T_print_len: {
        if (!(inst->operands[0].operand_type == Operand_register ||
              inst->operands[1].operand_type == Operand_register))
            inst->flags |= print_len_flag;
    } break;

    case T_imp_print_len: {
        if (identifier->val)
            inst->flags |= print_len_flag;
    } break;

    case T_arith_code: {
        inst->flags |= is_arith_flag;
        inst->operation += id_value + 1;
    } break;

    case T_imp_jmp: {
        if (identifier->val) {
            inst->flags |= is_jmp_flag;
            inst->operands[*operand_idx].operand_type = Operand_immediate;
            inst->operand_count = 1;
        }
    } break;
    }
}

instruction parse_instruction(mem_access *at) {
    uint8_t current_byte = access_mem(*at)[0];
    instruction inst = {};

    int lookup_idx = lookup_instruction(current_byte);
    if (lookup_idx == -1) {
        fprintf(stderr, "Could not recognize instruction: %x\n", current_byte);
    } else {
        instruction_format fmt = instruction_lookup[lookup_idx];
        instruction_bits *fmt_bits = fmt.Bits;

        inst.operation = fmt.Op;
        inst.operand_count = 2;

        uint8_t id_idx = 1;
        uint8_t bit_offset = fmt.Bits[0].size;

        bool check_disp = false;
        uint8_t operand_idx = 0;

        while (fmt_bits[id_idx].type != T_end) {
            instruction_bits identifier = fmt_bits[id_idx];

            decode_identifier(&inst, &identifier, current_byte, bit_offset,
                              &check_disp, &operand_idx, at);

            bit_offset += identifier.size;

            assert(bit_offset <= 8);
            if (bit_offset == 8) {
                at->offset++;
                current_byte = access_mem(*at)[0];
                bit_offset = 0;
            }

            if (bit_offset == 0 && check_disp) {
                uint8_t idx = operand_idx - 1;
                if (inst.operands[idx].operand_type == Operand_memory_8) {
                    int8_t D_Low = current_byte;

                    at->offset++;
                    current_byte = access_mem(*at)[0];

                    inst.operands[idx].memory.disp = D_Low;
                } else if (inst.operands[idx].operand_type ==
                               Operand_memory_16 ||
                           (inst.operands[idx].operand_type == Operand_memory &&
                            inst.operands[idx].memory.eff_addr ==
                                eff_add_direct)) {
                    int16_t D_Low = current_byte;

                    at->offset++;
                    current_byte = access_mem(*at)[0];
                    int16_t D_High = current_byte;

                    at->offset++;
                    current_byte = access_mem(*at)[0];

                    inst.operands[idx].memory.disp =
                        (D_High << 8) | (D_Low & 0xFF);
                }
                check_disp = false;
            }

            id_idx++;
        }
    }

    return inst;
}
