#include <libc.h>
#include "all.c"
#include "platform_metrics.c"
#include "simple_profiler.c"
#include "../lib/JsonParser/src/buffer.c"
#include "repetition_tester.c"
#include "read_overhead_test.c"

typedef struct {
    const char *name;
    ReadOverheadTestFunc *func;
} ReadTest;

ReadTest tests[] = {
    {"fread", &read_via_fread},
    /* {"read", &read_via_read}, */
};

int main(int argc, char **argv) {
    (void)&read_via_read;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [existing filename]\n", argv[0]);
    }

    char *filename = argv[1];

#if _WIN32
    struct __stat64 Stat;
    _stat64(filename, &Stat);
#else
    struct stat Stat;
    stat(filename, &Stat);
#endif
        
    Buffer dest = allocateBuffer(Stat.st_size);

    if (dest.count == 0) {
        fprintf(stderr, "File size 0, or error reading stats");
        return 1;
    }

    u64 cpu_timer_freq = get_cpu_freq(100);

    ReadTestParams params = {};
    params.filename = filename;
    params.dest = dest;

    while (true) {
        RepetitionTester tester_arr[ArraySize(tests) * AllocType_count] = {}; 

        for (int i = 0; i < ArraySize(tests); i++) {

            for (AllocationType alloc_type = 0; alloc_type < AllocType_count; 
                    alloc_type++) {
                RepetitionTester *tester= &tester_arr[i * AllocType_count + alloc_type];
                params.alloc_type = alloc_type;

                ReadTest test = tests[i];
                printf("\n--- %s%s%s ---\n", test.name, 
                        params.alloc_type ? " + " : "",
                        describe_allocation(params.alloc_type));
                new_test_wave(tester, params.dest.count, cpu_timer_freq, 10);
                test.func(tester, &params);
            }
        }
    }

    free_buffer(&dest);

    return 0;
}

