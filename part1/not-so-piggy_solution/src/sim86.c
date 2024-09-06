
static int read_file(mem_access at, char *src) {
    int32_t byte_length = -1;
    FILE *file = fopen(src, "rb");
    if (!file) {
        return byte_length;
    }

    uint32_t max_file_len = at.byte_count - at.base_address - at.offset;

    uint8_t *mem_offset = access_mem(at);
    byte_length = fread(mem_offset, sizeof(*mem_offset), max_file_len, file);
    fclose(file);

    return byte_length;
}

#define abs(a) (a < 0 ? -a : a)

static void print_jmp(FILE *out, operand op) {
    if (op.immediate + 2 >= 0)
        fprintf(out, "$+%d", op.immediate + 2);
    else
        fprintf(out, "$%d", op.immediate + 2);
}

static void print_mem(FILE *out, operand op, bool has_disp) {
    fprintf(out, "[");
    fprintf(out, "%s", memory_eff_addr_str_lookup[op.memory.eff_addr]);
    if (has_disp) {
        if (op.memory.eff_addr != eff_add_direct) {
            if (op.memory.disp < 0)
                fprintf(out, " - ");
            else
                fprintf(out, " + ");
        }
        fprintf(out, "%d", abs(op.memory.disp));
    }
    fprintf(out, "]");
}

static void print_imm(FILE *out, operand op) {
    fprintf(out, "%d", op.immediate);
}

static void print_reg(FILE *out, operand op) {
    register_ reg = op.reg;
    char *register_str = register_str_lookup[reg.val][reg.half];
    fprintf(out, "%s", register_str);
}

static void print_instruction(FILE *out, instruction inst) {
    char *operation_str = operation_lookup[inst.operation];

    fprintf(out, "%s ", operation_str);

    if (!(inst.flags & d_flag)) {
        operand temp = inst.operands[0];
        inst.operands[0] = inst.operands[1];
        inst.operands[1] = temp;
    }

    if (inst.flags & print_len_flag) {
        if (inst.flags & w_flag)
            fprintf(out, "word ");
        else
            fprintf(out, "byte ");
    }

    char *sep = "";
    for (int i = 0; i < inst.operand_count; i++) {
        fprintf(out, "%s", sep);
        sep = ", ";

        switch (inst.operands[i].operand_type) {
        case Operand_none:
            break;
        case Operand_register: {
            print_reg(out, inst.operands[i]);
        } break;
        case Operand_memory:
            if (inst.operands[i].memory.eff_addr == eff_add_direct)
                print_mem(out, inst.operands[i], 1);
            else
                print_mem(out, inst.operands[i], 0);
            break;
        case Operand_memory_8:
        case Operand_memory_16:
            print_mem(out, inst.operands[i], 1);
            break;
        case Operand_immediate: {
            if (inst.flags & is_jmp_flag) {
                print_jmp(out, inst.operands[i]);
            } else {
                print_imm(out, inst.operands[i]);
            }
        } break;
        }
    }
    /* fprintf(out, "\n"); */
}

void disassemble(FILE *out, int32_t bytes_len, mem_access *at) {
    int32_t count = bytes_len;
    while (count) {
        instruction ins = parse_instruction(at);
        if (ins.operation) {
            count -= at->offset;
            at->base_address += at->offset;
            at->offset = 0;
            print_instruction(out, ins);
        }
    }
}

void run_sim86(FILE *out, int32_t bytes_len, mem_access *at, bool simulate,
               bool dump, bool show_clocks) {
    state_8086 state = {};
    state.flag_reg = 0;
    state.main_memory.bytes = at->memory;
    state.main_memory.byte_count = at->byte_count;

    bool modified_regs[Register_count] = {};

    int16_t clocks = 0;

    int32_t count = bytes_len;
    while (count > at->offset + at->base_address) {
        instruction inst = parse_instruction(at);
        if (inst.operation) {
            at->base_address += at->offset;
            at->offset = 0;

            print_instruction(out, inst);

            if (simulate || show_clocks) {
                fprintf(out, "\t;");

                if (simulate) {
                    state.instruction_pointer = at->base_address + at->offset;
                    run_instruction(out, inst, &state, modified_regs);
                    at->base_address = state.instruction_pointer;
                }

                if (show_clocks) {
                    int16_t instruction_clocks = inst_clocks(inst);
                    assert(instruction_clocks != -1);
                    clocks += instruction_clocks;
                    fprintf(out, "  Clocks: +%d = %d",
                            instruction_clocks, clocks);
                }
            }


            fprintf(out, "\n");
        }
    }

    if (simulate) {
        fprintf(out, "\n");
        dump_registers(out, state, modified_regs);
    }

    if (dump) {
        FILE *mem_dump_file = fopen("sim86_memory_0.data", "wr");
        mem_dump(mem_dump_file, state.main_memory, 0,
                 state.main_memory.byte_count);
    }
}

int main(int argc, char **argv) {
    memory mem;

    bool run = false;
    bool simulate = false;
    bool dump = false;
    bool show_clocks = false;

    while (argc > 1) {
        run = true;
        if (strcmp(argv[1], "-exec") == 0)
            simulate = true;
        else if (strcmp(argv[1], "-dump") == 0)
            dump = true;
        else if (strcmp(argv[1], "-showclocks") == 0)
            show_clocks = true;
        else
            break;

        argc--;
        argv++;
    }

    if (!run) {
        fprintf(stderr, "Error! Need to provide arguments: (-exec) filename\n");
        return -1;
    }

    mem.bytes = malloc(sizeof(*mem.bytes) * 1 << MEM_SIZE_EXP);
    if (!mem.bytes) {
        fprintf(stderr, "Could not allocate 2^%d bytes of storage\n",
                MEM_SIZE_EXP);
        return 1;
    }
    mem.byte_count = 1 << MEM_SIZE_EXP;

    mem_access mem_access;
    mem_access.byte_count = mem.byte_count;
    mem_access.memory = mem.bytes;
    mem_access.mask = 0xffff;
    mem_access.base_address = 0;
    mem_access.offset = 0;

    if (argc > 1) {
        char *filename = argv[argc - 1];
        int32_t bytes_length = read_file(mem_access, filename);
        if (bytes_length == -1) {
            fprintf(stderr, "Could not read file %s\n", filename);
            return -1;
        }
        if (simulate) {
            fprintf(stdout, "; simulation of %s: \n", filename);
        } else {
            fprintf(stdout, "; dissasembly of %s: \n", filename);
            fprintf(stdout, "bits 16\n");
        }

        run_sim86(stdout, bytes_length, &mem_access, simulate, dump,
                  show_clocks);
    }

    return 0;
}
