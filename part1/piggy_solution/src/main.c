#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum { false, true } bool;
#define abs(a) ({ a < 0 ? -a : a; })

FILE *listing;

typedef enum {
    MEM_NO_DISP,
    MEM_8_DISP,
    MEM_16_DISP,
    REG,
} MOD;

typedef enum {
    MOV_RM_REG = 0b100010,
    MOV_IMM_RM = 0b1100011,
    MOV_IMM_REG = 0b1011,
    MOV_MEM_ACC = 0b1010000,
    MOV_ACC_MEM = 0b1010001,
    MOV_RM_SEGREG = 0b10001110,
    MOV_SEGREG_RM = 0b10001100,

    ARITH_RM_REG = 0b001110,
    ARITH_IMM_RM = 0b100000,
    ARITH_IMM_ACC = 0b0011110,

    JO = 0b01110000, // LOWEST
    JNO = 0b01110001,
    JB = 0b01110010,
    JAE = 0b01110011,
    JE = 0b01110100,
    JNE = 0b01110101,
    JBE = 0b01110110,
    JA = 0b01110111,
    JS = 0b01111000,
    JNS = 0b01111001,
    JP = 0b01111010,
    JPO = 0b01111011,
    JL = 0b01111100,
    JGE = 0b01111101,
    JLE = 0b01111110,
    JG = 0b01111111, // LARGEST

    LOOPNZ = 0b11100000, // LOWEST
    LOOPZ = 0b11100001,
    LOOP = 0b11100010,
    JCXZ = 0b11100011, // LARGEST
} OPCODE;

#define SMALLEST_J JO
#define LARGEST_J JG

#define SMALLEST_LOOP LOOPNZ
#define LARGEST_LOOP JCXZ

typedef enum {
    ARITH_RM_REG_RS = 1,
    ARITH_IMM_RM_RS = 3,
    ARITH_IMM_ACC_RS = 2,
} ARITHMETIC_PATTERN_RS;

typedef enum {
    ADD = 0b000,
    SUB = 0b101,
    CMP = 0b111,
} ARITHMETIC_PATTERN;

char *arithmetic_pattern_table[] = {"add", "", "", "", "", "sub", "", "cmp"};

typedef struct {
    uint8_t opcode;
    uint8_t code;

    bool w;
    bool d;
    bool s;
    MOD mode;
    uint8_t reg;
    uint8_t rm;

    uint8_t disp_low;
    uint8_t disp_high;

    int8_t data;
    int8_t data_w;
} Instruction;

char *jmp_table[] = {"jo", "jno", "jb", "jae", "je", "jne", "jbe", "ja",
                     "js", "jns", "jp", "jpo", "jl", "jge", "jle", "jg"};

char *loop_table[] = {"loopnz", "loopz", "loop", "jcxz"};

char *reg_table[] = {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
};

char *seg_reg_table[] = {"es", "cs", "ss", "ds"};

Instruction instruction_decode(uint8_t instruction_bytes[1]) {
    Instruction ret = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    bool get_data = false;
    ret.mode = 0b100;
    ret.s = 0;

    if (instruction_bytes[0] == MOV_RM_SEGREG ||
        instruction_bytes[0] == MOV_SEGREG_RM) {
        ret.opcode = instruction_bytes[0];
        ret.d = !((instruction_bytes[0] & 0b10) >> 1);

        uint8_t second_byte = 0;
        fread(&second_byte, 1, 1, listing);

        ret.mode = (second_byte & 0b11000000) >> 6;
        ret.reg = (second_byte & 0b00011000) >> 3;
        ret.rm = (second_byte & 0b00000111) >> 0;
    } else if (instruction_bytes[0] >> 2 == MOV_RM_REG) {
        ret.w = (instruction_bytes[0] & 0b1) >> 0;
        ret.d = (instruction_bytes[0] & 0b10) >> 1;
        ret.opcode = instruction_bytes[0] >> 2;

        uint8_t second_byte;
        fread(&second_byte, 1, 1, listing);

        ret.mode = (second_byte & 0b11000000) >> 6;
        ret.reg = (second_byte & 0b00111000) >> 3;
        ret.rm = (second_byte & 0b00000111) >> 0;
    } else if (instruction_bytes[0] >> 1 == MOV_IMM_RM) {
        ret.w = instruction_bytes[0] & 0b1;
        ret.opcode = instruction_bytes[0] >> 1;
        get_data = true;

        uint8_t second_byte;
        fread(&second_byte, 1, 1, listing);

        ret.mode = (second_byte & 0b11000000) >> 6;
        ret.rm = (second_byte & 0b00000111) >> 0;
    } else if (instruction_bytes[0] >> 4 == MOV_IMM_REG) {
        ret.w = (instruction_bytes[0] & 0b1000) >> 3;
        ret.reg = instruction_bytes[0] & 0b111;
        ret.opcode = instruction_bytes[0] >> 4;
        get_data = true;

        uint8_t second_byte = 0;
        fread(&second_byte, 1, 1, listing);

        ret.data = second_byte;
        ret.data_w = 0;
        if (ret.w) fread(&ret.data_w, 1, 1, listing);
    } else if (instruction_bytes[0] >> 2 == MOV_ACC_MEM >> 1) {
        // MOV (ACC - MEM/MEM - ACC)
        ret.w = instruction_bytes[0] & 0b1;
        ret.opcode = instruction_bytes[0] >> 1;
        ret.d = (instruction_bytes[0] & 0b10) >> 1;

        ret.mode = MEM_NO_DISP;
        ret.rm = 0b110;

        /* fread(&ret.disp_low, 1, 1, listing); */
        /* if (ret.w) fread(&ret.disp_high, 1, 1, listing); */
    } else if (instruction_bytes[0] >> 2 ==
               ((instruction_bytes[0] >> 2) & ARITH_RM_REG)) {
        ret.opcode = ARITH_RM_REG;
        ret.code = (instruction_bytes[0] & 0b00111000) >> 3;
        ret.d = (instruction_bytes[0] & 0b10) >> 1;
        ret.w = instruction_bytes[0] & 0b1;

        uint8_t second_byte = 0;
        fread(&second_byte, 1, 1, listing);

        ret.mode = second_byte >> 6;
        ret.reg = (second_byte & 0b00111000) >> 3;
        ret.rm = second_byte & 0b111;
    } else if (instruction_bytes[0] >> 2 == ARITH_IMM_RM) {
        ret.opcode = ARITH_IMM_RM;
        get_data = true;
        ret.s = (instruction_bytes[0] & 0b10) >> 1;
        ret.w = instruction_bytes[0] & 0b1;

        uint8_t second_byte = 0;
        fread(&second_byte, 1, 1, listing);

        ret.mode = second_byte >> 6;
        ret.code = (second_byte & 0b00111000) >> 3;
        ret.rm = second_byte & 0b111;
    } else if (instruction_bytes[0] >> 1 ==
               ((instruction_bytes[0] >> 1) & ARITH_IMM_ACC)) {
        ret.opcode = ARITH_IMM_ACC;
        ret.code = (instruction_bytes[0] & 0b00111000) >> 3;
        ret.w = instruction_bytes[0] & 0b1;
        get_data = true;
    } else if (instruction_bytes[0] >= SMALLEST_J &&
               instruction_bytes[0] <= LARGEST_J) {
        ret.opcode = SMALLEST_J;
        ret.code = instruction_bytes[0] & 0b1111;
        fread(&ret.data, 1, 1, listing);
    } else if (instruction_bytes[0] >= SMALLEST_LOOP &&
               instruction_bytes[0] <= LARGEST_LOOP) {
        ret.opcode = SMALLEST_LOOP;
        ret.code = instruction_bytes[0] & 0b11;
        fread(&ret.data, 1, 1, listing);
    }

    if (ret.mode != 0b100) {
        uint8_t disp[2] = {0, 0};
        switch (ret.mode) {
        case MEM_8_DISP:
            assert(fread(&disp[0], 1, 1, listing));
            ret.disp_low = disp[0];
            break;
        case MEM_NO_DISP:
            if (ret.rm != 0b110) break;
        case MEM_16_DISP:
            assert(fread(disp, 2, 1, listing));
            ret.disp_low = disp[0];
            ret.disp_high = disp[1];
            break;
        case REG:
            break;
        }
    }

    if (get_data) {
        fread(&ret.data, 1, 1, listing);
        if (ret.w && !ret.s) fread(&ret.data_w, 1, 1, listing);
    }

    return ret;
}

void get_rm(Instruction instruction, char rm_str[40]) {
    switch (instruction.mode) {
    case REG: {
        int rm_idx = instruction.rm;
        if (instruction.w) rm_idx += 8;
        strcpy(rm_str, reg_table[rm_idx]);
    } break;
    case MEM_NO_DISP:
    case MEM_8_DISP:
    case MEM_16_DISP:
        strcpy(rm_str, "[");
        if (!(instruction.rm & 0b100)) {
            // (BX/BP) + (SI/DI) + ( /D8/D16)
            int bx_bp_idx = 2 * ((instruction.rm & 0b010) >> 1);
            bx_bp_idx += 11;
            strcat(rm_str, reg_table[bx_bp_idx]);

            strcat(rm_str, " + ");

            int si_di_idx = instruction.rm & 0b1;
            si_di_idx += 14;
            strcat(rm_str, reg_table[si_di_idx]);
        } else if (!(instruction.rm & 0b010)) {
            // (SI/DI) + ( /D8/D16)
            int si_di_idx = instruction.rm & 0b1;
            si_di_idx += 14;
            strcat(rm_str, reg_table[si_di_idx]);
        } else if (!(instruction.rm & 0b001)) {
            //  DIRECT ADRESS/(BP) + ( /D8/D16)
            if (instruction.mode == MEM_NO_DISP) {
                int16_t direct_address_int =
                    (instruction.disp_high << 8) |
                    (instruction.disp_low & 0b11111111);
                char direct_address[40];
                sprintf(direct_address, "%i", direct_address_int);
                strcat(rm_str, direct_address);
            } else {
                strcat(rm_str, "bp");
            }
        } else {
            // (BX)
            strcat(rm_str, "bx");
        }

        switch (instruction.mode) {
        case REG:
        case MEM_NO_DISP:
            break;
        case MEM_8_DISP: {
            int8_t d8_int = instruction.disp_low;
            if (d8_int == 0) break;
            char d8[40];
            if (d8_int >= 0)
                sprintf(d8, " + %i", d8_int);
            else
                sprintf(d8, " - %i", abs(d8_int));
            strcat(rm_str, d8);
        } break;
        case MEM_16_DISP: {
            int16_t d16_int = (instruction.disp_high << 8) |
                              (instruction.disp_low & 0b11111111);
            if (d16_int == 0) break;
            char d16[40];
            if (d16_int >= 0)
                sprintf(d16, " + %i", d16_int);
            else
                sprintf(d16, " - %i", abs(d16_int));
            strcat(rm_str, d16);
        } break;
        }
        strcat(rm_str, "]");
        break;
    }
}

void print_op(Instruction instruction) {
    switch (instruction.opcode) {
    case MOV_RM_REG: {
        fprintf(stdout, "mov ");

        char reg[40] = "";
        int reg_idx = instruction.reg;
        if (instruction.w) reg_idx += 8;
        strcpy(reg, reg_table[reg_idx]);

        char rm[40] = "";
        get_rm(instruction, rm);

        char *s = reg;
        char *d = rm;
        if (instruction.d) {
            s = rm;
            d = reg;
        }

        fprintf(stdout, "%s, %s\n", d, s);
    } break;
    case MOV_IMM_RM:
        fprintf(stdout, "mov ");

        char rm[40] = "";
        get_rm(instruction, rm);

        char data[40] = "";
        if (instruction.w)
            sprintf(data, "word %i",
                    (instruction.data_w << 8) |
                        (instruction.data & 0b11111111));
        else
            sprintf(data, "byte %i", (int8_t)instruction.data);

        fprintf(stdout, "%s, %s\n", rm, data);
        break;
    case MOV_IMM_REG: {
        fprintf(stdout, "mov ");

        char reg[40] = "";
        int reg_idx = instruction.reg;
        if (instruction.w) reg_idx += 8;
        strcpy(reg, reg_table[reg_idx]);

        char imm[40] = "";
        int16_t imm_int = 0;
        if (instruction.w)
            imm_int =
                (instruction.data_w << 8) | (instruction.data & 0b11111111);
        else
            imm_int = (int8_t)instruction.data;

        sprintf(imm, "%i", imm_int);

        fprintf(stdout, "%s, %s\n", reg, imm);
    } break;
    case MOV_ACC_MEM:
    case MOV_MEM_ACC: {
        fprintf(stdout, "mov ");

        char reg[40] = "ax";

        char rm[40] = "";
        get_rm(instruction, rm);

        char *s = rm, *d = reg;
        if (instruction.d) {
            s = reg;
            d = rm;
        }

        fprintf(stdout, "%s, %s\n", d, s);
    } break;
    case MOV_RM_SEGREG:
    case MOV_SEGREG_RM: {
        fprintf(stdout, "mov ");

        char segreg[40] = "";
        strcpy(segreg, seg_reg_table[instruction.reg]);

        char rm[40] = "";
        get_rm(instruction, rm);

        char *s = rm, *d = segreg;
        if (instruction.d) {
            s = segreg;
            d = rm;
        }

        fprintf(stdout, "%s, %s\n", d, s);
    } break;
    case ARITH_RM_REG: {
        fprintf(stdout, "%s ", arithmetic_pattern_table[instruction.code]);
        char reg[40] = "";
        int reg_idx = instruction.reg;
        if (instruction.w) reg_idx += 8;
        strcpy(reg, reg_table[reg_idx]);

        char rm[40] = "";
        get_rm(instruction, rm);

        char *s = reg, *d = rm;
        if (instruction.d) {
            s = rm;
            d = reg;
        }

        printf("%s, %s\n", d, s);
    } break;
    case ARITH_IMM_RM: {
        fprintf(stdout, "%s ", arithmetic_pattern_table[instruction.code]);

        char rm[40] = "";
        get_rm(instruction, rm);

        if (instruction.mode != REG) {
            char new[40];
            if (instruction.w)
                sprintf(new, "word %s", rm);
            else
                sprintf(new, "byte %s", rm);
            strcpy(rm, new);
        }

        char data[40] = "";
        if (instruction.w)
            sprintf(data, "%i",
                    (instruction.data_w << 8) |
                        (instruction.data & 0b11111111));
        else
            sprintf(data, "%i", (int8_t)instruction.data);

        fprintf(stdout, "%s, %s\n", rm, data);
        break;
    } break;
    case ARITH_IMM_ACC: {
        fprintf(stdout, "%s ", arithmetic_pattern_table[instruction.code]);

        if (instruction.w)
            fprintf(stdout, "ax, %i\n",
                    (instruction.data_w << 8) |
                        (instruction.data & 0b11111111));
        else
            fprintf(stdout, "al, %i\n", (int8_t)instruction.data);
    } break;
    case SMALLEST_J: {
        fprintf(stdout, "%s ", jmp_table[instruction.code]);
        if (instruction.data + 2 >= 0)
            fprintf(stdout, "$+%d\n", instruction.data + 2);
        else
            fprintf(stdout, "$%d\n", instruction.data + 2);
    } break;
    case SMALLEST_LOOP: {
        fprintf(stdout, "%s ", loop_table[instruction.code]);
        if (instruction.data + 2 >= 0)
            fprintf(stdout, "$+%d\n", instruction.data + 2);
        else
            fprintf(stdout, "$%d\n", instruction.data + 2);
    } break;
    }
}

int main(int argc, char **argv) {
    char listing_name[50] = "";
    /* strcpy(listing_name, "listings/"); */
    if (argc > 1)
        strcpy(&listing_name[strlen(listing_name)], argv[1]);
    else
        return 1;

    listing = fopen(listing_name, "r+");
    if (!listing) {
        fprintf(stderr, "Error opening listing: %s\n", listing_name);
        return 1;
    }

    printf("; %s:\nbits 16\n\n", listing_name);

    uint8_t instruction_bytes[1] = {0};
    while (fread(instruction_bytes, 1, 1, listing)) {
        Instruction instruction = instruction_decode(instruction_bytes);
        print_op(instruction);
    }

    fclose(listing);
}
