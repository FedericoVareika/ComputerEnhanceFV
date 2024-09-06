#pragma once

typedef struct memory {
    uint8_t *bytes;
    uint32_t byte_count;
} memory;

typedef struct mem_access {
    uint8_t *memory;
    uint32_t mask;

    uint32_t base_address;
    uint32_t offset;

    uint32_t byte_count;
} mem_access;

uint8_t *access_mem(mem_access at);

void mem_dump(FILE *out, memory mem, uint32_t at, uint32_t len);

