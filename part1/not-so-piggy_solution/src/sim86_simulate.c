typedef enum {
    Undefined,
    Both,
    Lower,
    Higher,
} Half;

int16_t get_mask(Half half) {
    switch (half) {
    case Undefined:
        return -1;
    case Lower:
        return 0x00ff;
    case Higher:
        return 0xff00;
    case Both:
        return 0xffff;
    }
}

typedef struct {
    enum {
        Reg_access,
        Mem_access,
    } op_val_enum;

    union {
        int16_t *reg_access;
        int8_t *mem_access;
    };
} OpAccess;

int16_t get_op_val(operand op, OpAccess op_access, Half half) {
    uint16_t mask = get_mask(half);
    if (op.operand_type == Operand_immediate) 
        return op.immediate & mask;

    switch (op_access.op_val_enum) {
    case Reg_access: 
        return *op_access.reg_access & mask;
    case Mem_access: {
         int16_t op_val = (int16_t)op_access.mem_access[0] & 0xff;
         if (half == Both)
             op_val = (op_val & 0xff) | 
                 (((int16_t)op_access.mem_access[1] << 8) & 0xff00);
         return op_val & mask;
    }
    }
}

void set_op_val(OpAccess op_access, int16_t op_val, Half half) {
    switch (op_access.op_val_enum) {
    case Reg_access: {
        uint16_t mask = 0x00ff;
        switch (half) {
        case Undefined:
            break;
        case Both:
            *op_access.reg_access = op_val;
            break;
        case Higher:
            op_val <<= 8;
            mask = 0xff00;
        case Lower:
            *op_access.reg_access &= ~mask;
            *op_access.reg_access |= op_val & mask;
            break;
        }
    } break;
    case Mem_access:
        switch (half) {
        case Undefined:
        case Higher: 
            break;
        case Both: 
            op_access.mem_access[1] = (int8_t)((op_val & 0xff00) >> 8);
        case Lower:
            op_access.mem_access[0] = (int8_t)op_val & 0xff;
            break;
        }
        break;
    }
}

uint8_t *get_mem_addr(state_8086 *state, operand op, int16_t disp) {
    uint16_t addr; 
    switch (op.memory.eff_addr) {
    case eff_add_bx_si: 
        addr = state->registers[Register_b] +
            state->registers[Register_si];
        break;
    case eff_add_bx_di: 
        addr = state->registers[Register_b] +
            state->registers[Register_di];
        break;
    case eff_add_bp_si: 
        addr = state->registers[Register_bp] +
            state->registers[Register_si];
        break;
    case eff_add_bp_di: 
        addr = state->registers[Register_bp] +
            state->registers[Register_di];
        break;

    case eff_add_si:
        addr = state->registers[Register_si];
        break;
    case eff_add_di:
        addr = state->registers[Register_di];
        break;
    case eff_add_bx:
        addr = state->registers[Register_b];
        break;
    case eff_add_bp:
        addr = state->registers[Register_bp];
        break;

    case eff_add_direct: 
        addr = 0;
        break;
    } 

    addr += disp;
    uint32_t wrap = state->main_memory.byte_count;
    return &state->main_memory.bytes[addr % wrap];
}

Half get_operand(state_8086 *state, operand op, OpAccess *dst, char **str) {
    switch (op.operand_type) {
    case Operand_none:
        break;

    case Operand_register: {
        dst->op_val_enum = Reg_access;
        dst->reg_access = &(state->registers[op.reg.val]);
        if (str)
            *str = register_str_lookup[op.reg.val][op.reg.half];
        if (op.reg.half == None) {
            return Both;
        } else if (op.reg.half == Lower_half) {
            return Lower;
        } else if (op.reg.half == Higher_half) {
            return Higher;
        }
    } break;

    case Operand_immediate: {
        return Undefined;
        /* *dst = &(op.immediate); */
    } break;

    case Operand_memory_16: 
    case Operand_memory_8: 
        dst->op_val_enum = Mem_access;
        dst->mem_access = (int8_t *)get_mem_addr(state, op, op.memory.disp);
        return Undefined;

    case Operand_memory: 
        dst->op_val_enum = Mem_access;
        dst->mem_access = (int8_t *)get_mem_addr(state, op, op.memory.disp);
        return Undefined;
    }

    return Undefined;
}

void print_flags(FILE *out, uint16_t flag_reg) {
    if (flag_reg & CF)
        fprintf(out, "C");
    if (flag_reg & PF)
        fprintf(out, "P");
    if (flag_reg & AF)
        fprintf(out, "A");
    if (flag_reg & ZF)
        fprintf(out, "Z");
    if (flag_reg & SF)
        fprintf(out, "S");
    if (flag_reg & TF)
        fprintf(out, "T");
    if (flag_reg & IF)
        fprintf(out, "I");
    if (flag_reg & DF)
        fprintf(out, "D");
    if (flag_reg & OF)
        fprintf(out, "O");
}

int parity(int16_t val) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        count += (val >> i) & 1;
    }
    return count % 2;
}

typedef struct {
    bool update_flag, half, sub;
    int16_t src, dest, res;
    state_8086 *state;
} Update_flag_args;

void update_flags(Update_flag_args args) {
    state_8086 *state = args.state;
    int16_t src_adj = args.src;
    if (args.sub) {
        src_adj = ~args.src + 1;
    }

    // Zero
    if (!args.res)
        state->flag_reg |= ZF;
    else
        state->flag_reg &= ~ZF;

    // Sign
    if (args.res < 0)
        state->flag_reg |= SF;
    else
        state->flag_reg &= ~SF;

    // Parity
    if (!parity(args.res))
        state->flag_reg |= PF;
    else
        state->flag_reg &= ~PF;

    // Overflow
    if (args.half) {
        if ((src_adj & 0x80) == (args.dest & 0x80) &&
            (src_adj & 0x80) != (args.res & 0x80))
            state->flag_reg |= OF;
        else
            state->flag_reg &= ~OF;
    } else {
        if ((src_adj & 0x8000) == (args.dest & 0x8000) &&
            (src_adj & 0x8000) != (args.res & 0x8000))
            state->flag_reg |= OF;
        else
            state->flag_reg &= ~OF;
    }

    // Carry
    {
        int32_t src_wide = (int32_t)args.src & 0xffff;
        int32_t dest_wide = (int32_t)args.dest & 0xffff;
        int32_t res_wide;
        if (args.sub)
            res_wide = dest_wide - src_wide;
        else
            res_wide = dest_wide + src_wide;

        if ((args.half && res_wide & 0xff00) ||
            (!args.half && res_wide & 0xffff0000))
            state->flag_reg |= CF;
        else
            state->flag_reg &= ~CF;
    }

    // Auxiliary carry flag
    {
        int32_t src_nibble = (int32_t)args.src & 0xf;
        int32_t dest_nibble = (int32_t)args.dest & 0xf;
        int32_t res_nibble;
        if (args.sub)
            res_nibble = dest_nibble - src_nibble;
        else
            res_nibble = dest_nibble + src_nibble;

        if (res_nibble & 0xf0)
            state->flag_reg |= AF;
        else
            state->flag_reg &= ~AF;
    }
}

#define get_flag(flag_reg, flag) (((flag_reg) & (flag)) > 0)

int jmp_valid(instruction inst, int16_t flag_reg, int16_t cx) {
    switch (inst.operation) {
    case Op_je: 
        return get_flag(flag_reg, ZF);

    case Op_jne: 
        return !get_flag(flag_reg, ZF);

    case Op_jl: 
        return get_flag(flag_reg, SF) != get_flag(flag_reg, OF);

    case Op_jnl: 
        return !(get_flag(flag_reg, SF) != get_flag(flag_reg, OF));

    case Op_jle: 
        return get_flag(flag_reg, ZF) 
            || (get_flag(flag_reg, SF) != get_flag(flag_reg, OF));

    case Op_jnle: 
        return !(get_flag(flag_reg, ZF)
                || (get_flag(flag_reg, SF) != get_flag(flag_reg, OF)));

    case Op_jb: 
        return get_flag(flag_reg, CF); 

    case Op_jnb: 
        return !get_flag(flag_reg, CF); 

    case Op_jbe: 
        return get_flag(flag_reg, CF) || get_flag(flag_reg, ZF); 

    case Op_jnbe: 
        return !(get_flag(flag_reg, CF) || get_flag(flag_reg, ZF)); 

    case Op_jp: 
        return get_flag(flag_reg, PF);

    case Op_jnp: 
        return !get_flag(flag_reg, PF);

    case Op_jo: 
        return get_flag(flag_reg, OF);

    case Op_jno: 
        return !get_flag(flag_reg, OF);

    case Op_js:
        return get_flag(flag_reg, SF);

    case Op_jns: 
        return !get_flag(flag_reg, SF);

    case Op_loop: 
        return cx != 0;
    case Op_loopz: 
        return cx != 0 && get_flag(flag_reg, ZF);
    case Op_loopnz: 
        return cx != 0 && !get_flag(flag_reg, ZF);
    case Op_jcxz: 
        return cx == 0;

    default: 
        return false;
    }
}

void process_operation(instruction inst, state_8086 *state, int16_t dest_val,
                        int16_t src_val, Half half, OpAccess dest_access,
                        bool *print_change) {
    Update_flag_args flag_args = {};
    flag_args.state = state;
    flag_args.src = src_val;
    flag_args.dest = dest_val;
    flag_args.half = half != Both;
    flag_args.sub = false;

    switch (inst.operation) {
    case Op_mov: {
        set_op_val(dest_access, src_val, half);
    } break;

    case Op_add: {
        set_op_val(dest_access, dest_val + src_val, half);

        flag_args.res = *dest_access.reg_access;
        update_flags(flag_args);
    } break;

    case Op_sub: {
        set_op_val(dest_access, dest_val - src_val, half);

        flag_args.res = *dest_access.reg_access;
        flag_args.sub = true;
        update_flags(flag_args);
    } break;

    case Op_cmp: {
        *print_change = false;

        flag_args.res = dest_val - src_val;
        flag_args.sub = true;
        update_flags(flag_args);
    } break;

    default: 
        break;
    }
}

void loop_instruction(state_8086 *state) {
    state->registers[Register_c]--;
}

void run_instruction(FILE *out, instruction inst, state_8086 *state,
                      bool modified_regs[Register_count]) {
    bool print_change = false;
    char *dest_str = NULL;

    OpAccess dest_access;
    int16_t dest_val;
    int16_t flag_reg_before = state->flag_reg;

    if (inst.flags & is_jmp_flag) {
        assert(inst.operand_count == 1);
        assert(inst.operands[0].operand_type == Operand_immediate);

        int16_t jmp_dif = inst.operands[0].immediate;
        dest_str = "ip";

        dest_access.op_val_enum = Reg_access;
        dest_access.reg_access = (int16_t *)&state->instruction_pointer;
        dest_val = state->instruction_pointer;

        if (inst.operation >= Op_loop && inst.operation <= Op_jcxz) 
            loop_instruction(state);
        

        if (jmp_valid(inst, state->flag_reg, state->registers[Register_c]))
            state->instruction_pointer += jmp_dif;

        print_change = true;
    } else {
        int dest_idx = 1, src_idx = 0;
        if (inst.flags & d_flag) {
            dest_idx = 0;
            src_idx = 1;
        }

        operand dest_op = inst.operands[dest_idx];
        if (dest_op.operand_type == Operand_register) {
            modified_regs[dest_op.reg.val] = true;
        }

        operand src_op = inst.operands[src_idx];

        Half dest_half = 
            get_operand(state, dest_op, &dest_access, &dest_str);
        if (dest_half == Undefined) 
            dest_half = inst.flags & w_flag ? Both : Lower;

        OpAccess src_access;
        Half src_half = 
            get_operand(state, src_op, &src_access, NULL);
        if (src_half == Undefined) 
            src_half = inst.flags & w_flag ? Both : Lower;

        dest_val = get_op_val(dest_op, dest_access, dest_half);
        int16_t src_val = get_op_val(src_op, src_access, src_half);
        
        Update_flag_args flag_args = {};
        flag_args.state = state; 
        
        print_change = true;
        process_operation(inst, state, dest_val, src_val, dest_half,
                            dest_access, &print_change);
    }

    if (print_change) {
        fprintf(out, "  %s: 0x%04x->0x%04x", dest_str, dest_val & 0xffff,
                *dest_access.reg_access & 0xffff);
    }

    if (flag_reg_before != state->flag_reg) {
        fprintf(out, "  flag:");
        print_flags(out, flag_reg_before);
        fprintf(out, "->");
        print_flags(out, state->flag_reg);
    }
}

void dump_registers(FILE *out, state_8086 state,
                    bool modified_regs[Register_count]) {
    fprintf(out, "  Registers: \n");
    for (int i = 0; i < Register_count; i++) {
        if (modified_regs[i])
            fprintf(out, "%10s: 0x%04x\n", register_str_lookup[i][2],
                    state.registers[i] & 0xffff);
    }

    fprintf(out, "%10s: 0x%04x\n", "ip", state.instruction_pointer & 0xffff);

    fprintf(out, "%10s: ", "flags");
    print_flags(out, state.flag_reg);
    fprintf(out, "\n");
}
