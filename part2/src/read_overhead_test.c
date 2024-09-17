
typedef enum {
    AllocType_none,
    AllocType_malloc,

    AllocType_count,
} AllocationType;

typedef struct {
    char *filename;
    Buffer dest;
    AllocationType alloc_type;
} ReadTestParams;

const char *describe_allocation(AllocationType alloc_type) {
    switch (alloc_type) {
    case AllocType_none: 
        return "";
    case AllocType_malloc:
        return "malloc";
    default:
        return "UNKNOWN";
    }
}

void handle_allocation(Buffer *dest, AllocationType alloc_type) {
    switch (alloc_type) {
    case AllocType_none:
        break;
    case AllocType_malloc:
        *dest = allocateBuffer(dest->count);
        break;
    default: 
        fprintf(stderr, "ERROR: Unrecognized allocation type");
        break;
    }
}

void handle_deallocation(Buffer *dest, AllocationType alloc_type) {
    switch (alloc_type) {
    case AllocType_none:
        break;
    case AllocType_malloc:
        free_buffer(dest);
        break;
    default: 
        fprintf(stderr, "ERROR: Unrecognized deallocation type");
        break;
    }
}

typedef void ReadOverheadTestFunc(RepetitionTester *tester, 
                                  ReadTestParams *params);

static void read_via_fread(RepetitionTester *tester, ReadTestParams *params) {
    while (is_testing(tester)) {
        FILE *file = fopen(params->filename, "rb");

        if (!file) {
            error(tester, "Fopen failed");
            continue;
        } 

        Buffer dest = params->dest;
        handle_allocation(&dest, params->alloc_type);
        
        begin_time(tester);

        size_t result = fread(dest.data, dest.count, 1, file);

        end_time(tester);

        if (result == 1) {
            count_bytes(tester, dest.count);
        } else {
            error(tester, "Fread failed");
        }

        handle_deallocation(&dest, params->alloc_type);

        fclose(file);
    }
}

static void read_via_read(RepetitionTester *tester, ReadTestParams *params) {
    while (is_testing(tester)) {
        int fd = open(params->filename, O_RDONLY);

        if (fd < 0) {
            error(tester, "Open failed");
            continue;
        } 

        Buffer dest = params->dest;
        handle_allocation(&dest, params->alloc_type);
        
        begin_time(tester);

        size_t result = read(fd, dest.data, dest.count);

        end_time(tester);

        if (result > 0) {
            assert(result == dest.count);
            count_bytes(tester, dest.count);
        } else {
            error(tester, "Read failed");
        }

        handle_deallocation(&dest, params->alloc_type);

        close(fd);
    }
}
