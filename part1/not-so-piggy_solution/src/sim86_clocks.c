#include "sim86_clocks.h"

#define disp_clocks 4

int16_t ea_clock_table[] = {
        7, 8, 8, 7,
        5, 5, 5, 5,
        6,
};

int16_t ea_clocks(operand_type op_type, effective_address_enum eff_addr,
                  int16_t disp) {
    int16_t ea_clocks = ea_clock_table[eff_addr]; 
    if (op_type != Operand_memory && disp != 0)
        ea_clocks += disp_clocks;
    return ea_clocks;
}

int16_t inst_clocks(instruction inst) {
    int8_t src_idx = 0;
    int8_t dest_idx = 1;
    if (inst.flags & d_flag) {
        src_idx = 1;
        dest_idx = 0;
    }

    operand dest = inst.operands[dest_idx];
    operand src = inst.operands[src_idx];
    switch (inst.operation) {
    case Op_mov: 
        switch (dest.operand_type) {
        case Operand_register: 
            switch (src.operand_type) {
            case Operand_register:
                return 2;
            case Operand_memory: 
            case Operand_memory_8: 
            case Operand_memory_16: 
                return 8 + ea_clocks(src.operand_type, src.memory.eff_addr, 
                                     src.memory.disp);

            case Operand_immediate: 
                return 4;
            default:
                return -1;
            }

        case Operand_memory: 
        case Operand_memory_8: 
        case Operand_memory_16: 
            switch (src.operand_type) {
            case Operand_register:
                if (src.reg.val == Register_a) 
                    return 10;
                else 
                    return 9 + 
                        ea_clocks(dest.operand_type, dest.memory.eff_addr,
                                  dest.memory.disp);
            case Operand_immediate: 
                return 10 + ea_clocks(dest.operand_type, dest.memory.eff_addr,
                                      dest.memory.disp);
            default:
                return -1;
            }
        default:
            return -1;
        }

        case Operand_register: 
            switch (src.operand_type) {
            case Operand_register: 
                return 2;
            case Operand_memory: 
            case Operand_memory_8: 
            case Operand_memory_16: 
                if (dest.reg.val == Register_a)
                    return 10;
                else 
                    return 8 +
                        ea_clocks(src.operand_type, src.memory.eff_addr,
                                  src.memory.disp);
            case Operand_immediate: 
                return 4;
            default: 
                return -1;
            }

        default: 
            return -1;
    case Op_add: 
    case Op_sub: 
        switch (dest.operand_type) {
        case Operand_register: 
            switch (src.operand_type) {
            case Operand_register: 
                return 3;
            case Operand_memory: 
            case Operand_memory_8: 
            case Operand_memory_16: 
                return 9 + ea_clocks(src.operand_type, src.memory.eff_addr,
                                     src.memory.disp);
            case Operand_immediate: 
                return 4;
            default: 
                return -1;
            }
        case Operand_memory: 
        case Operand_memory_8: 
        case Operand_memory_16: 
            switch (src.operand_type) {
            case Operand_register: 
                return 16 + ea_clocks(dest.operand_type, dest.memory.eff_addr,
                                      dest.memory.disp);
            case Operand_immediate: 
                return 17 + ea_clocks(dest.operand_type, dest.memory.eff_addr,
                                      dest.memory.disp);
            default:
                return -1;
            }

        default: 
            return -1;
        }
     
    case Op_cmp: 
        switch (dest.operand_type) {
        case Operand_register: 
            switch (src.operand_type) {
            case Operand_register: 
                return 3;
            case Operand_memory: 
            case Operand_memory_8: 
            case Operand_memory_16: 
                return 9 + ea_clocks(src.operand_type, src.memory.eff_addr,
                                     src.memory.disp);
            case Operand_immediate: 
                return 4;
            default: 
                return -1;
            }
        case Operand_memory: 
        case Operand_memory_8: 
        case Operand_memory_16: 
            switch (src.operand_type) {
            case Operand_register: 
                return 9 + ea_clocks(dest.operand_type, dest.memory.eff_addr,
                                     dest.memory.disp);
            case Operand_immediate: 
                return 10 + ea_clocks(dest.operand_type, dest.memory.eff_addr,
                                      dest.memory.disp);
            default:
                return -1;
            }

        default: 
            return -1;
        }
    }
}
