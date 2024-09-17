
typedef enum {
    TestMode_Uninitialized, 
    TestMode_Testing, 
    TestMode_Completed, 
    TestMode_Error, 
} TestMode;

typedef struct {
    u32 test_count;
    u64 total_time;
    u64 min_time;
    u64 max_time;
} RepetitionTestResults;

typedef struct {
    u64 target_byte_count;
    u64 cpu_timer_freq;
    u64 try_for_time;
    u64 tests_started_at;

    TestMode mode;
    bool print_new_minimums;
    u32 open_block_count;
    u32 close_block_count;
    u64 time_accumulated_on_this_test;
    u64 bytes_accumulated_on_this_test;

    RepetitionTestResults results;
} RepetitionTester;

f64 seconds_from_cpu_time(u64 cpu_time, u64 cpu_timer_freq) {
    assert(cpu_timer_freq);
    return ((f64)cpu_time / (f64)cpu_timer_freq); 
}

static void print_time(const char *label, u64 cpu_time, u64 cpu_timer_freq, 
                u64 byte_count) {
    printf("%s: %llu", label, cpu_time);
    if (cpu_timer_freq) {
        f64 seconds = seconds_from_cpu_time(cpu_time, cpu_timer_freq);
        printf(" (%fms", 1000 * seconds);

        if (byte_count) {
            f64 gigabyte = 1024.0f * 1024.0f * 1024.0f;
            f64 gigabytes_per_second = byte_count / (gigabyte * seconds);
            printf(", %fgb/s", gigabytes_per_second);
        }
        printf(")");
    }
    fflush(stdout);
}

void print_results(RepetitionTestResults results, u64 cpu_timer_freq, 
                   u64 byte_count) {
    print_time("Max", results.max_time, cpu_timer_freq, byte_count);
    printf("\n");
    print_time("Min", results.min_time, cpu_timer_freq, byte_count);
    printf("\n");

    if (results.test_count) {
        u64 average_time = results.total_time / results.test_count;
        print_time("Avg", average_time, cpu_timer_freq, byte_count);
    }
}

void new_test_wave(RepetitionTester *tester, u64 target_byte_count, 
                   u64 cpu_timer_freq, u32 seconds_to_try) {
    if (tester->mode == TestMode_Uninitialized) {
        tester->mode = TestMode_Testing;
        tester->target_byte_count = target_byte_count;
        tester->cpu_timer_freq = cpu_timer_freq;
        tester->print_new_minimums = true;
        tester->results.min_time = UINT64_MAX;
    } else if (tester->mode == TestMode_Completed) {
        tester->mode = TestMode_Testing;
    }

    tester->try_for_time = seconds_to_try * cpu_timer_freq;
    tester->tests_started_at = read_cpu_timer();
}

void begin_time(RepetitionTester *tester) {
    tester->open_block_count++;
    tester->time_accumulated_on_this_test -= read_cpu_timer();
}

void end_time(RepetitionTester *tester) {
    tester->close_block_count++;
    tester->time_accumulated_on_this_test += read_cpu_timer();
}

static void error(RepetitionTester *tester, char const *message) {
    tester->mode = TestMode_Error;
    fprintf(stderr, "ERROR: %s\n", message);
}

bool is_testing(RepetitionTester *tester) {
    if (tester->mode != TestMode_Testing)
        return false;

    if (!tester->open_block_count)
        return true;

    u64 current_time = read_cpu_timer();

    if (tester->open_block_count != tester->close_block_count) {
        error(tester, "Unbalanced block counts");
    }

    if (tester->bytes_accumulated_on_this_test != tester->target_byte_count) {
        error(tester, "Process byte count mismatch:");
        fprintf(stderr,
                "%llu != %llu", 
                tester->bytes_accumulated_on_this_test,
                tester->target_byte_count);
    }

    if (tester->mode != TestMode_Testing) 
        return false;

    if((current_time - tester->tests_started_at) > tester->try_for_time) {
        tester->mode = TestMode_Completed;

        printf("                                                          \r");
        print_results(tester->results, tester->cpu_timer_freq,
                tester->target_byte_count);

        return false;
    } else {
        /* printf(" (%llu)", current_time - tester->tests_started_at); */
        /* fflush(stdout); */
        /* printf("\r"); */
    }
    
    RepetitionTestResults *results = &tester->results; 
    results->test_count++;
    u64 elapsed = tester->time_accumulated_on_this_test;
    results->total_time += elapsed;
    
    if (results->max_time < elapsed) {
        results->max_time = elapsed;
    }

    if (results->min_time > elapsed) {
        results->min_time = elapsed;
        tester->tests_started_at = current_time;

        if(tester->print_new_minimums) {
            /* printf("                                            \r"); */
            print_time("Min", results->min_time, tester->cpu_timer_freq,
                       tester->bytes_accumulated_on_this_test);
            printf("               \r");
        }
    }


    tester->open_block_count = 0;
    tester->close_block_count = 0;
    tester->time_accumulated_on_this_test = 0;
    tester->bytes_accumulated_on_this_test = 0;

    return true;
}

void count_bytes(RepetitionTester *tester, u64 count) {
    tester->bytes_accumulated_on_this_test += count;
}
