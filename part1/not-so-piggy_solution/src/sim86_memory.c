#include "sim86_memory.h"

uint8_t *access_mem(mem_access at) {
    return &at.memory[at.base_address + at.offset & at.mask];
}

void mem_dump(FILE *out, memory mem, uint32_t at, uint32_t len) {
     fwrite(&mem.bytes[at], sizeof(*mem.bytes), len, out);
}
