#define EARTH_RADIUS 6372.8

typedef struct {
    f64 x0, y0;
    f64 x1, y1;
} Pair;

u32 parse_haversine(Pair *haversine_pairs, Buffer *json_buf, u64 max_pairs) {
    u32 pair_count = 0;

    JsonElement JSON = parse_json(json_buf);
    JsonElement *pairs = lookup_json_element(&JSON, stringLiteral("pairs"));

    JsonElement *pair_elem = pairs->first_son;
    while (pair_elem) {
        Pair *pair = &haversine_pairs[pair_count];
        *pair = (Pair){
            lookup_json_element(pair_elem, stringLiteral("x0"))->value.f64,
            lookup_json_element(pair_elem, stringLiteral("y0"))->value.f64,
            lookup_json_element(pair_elem, stringLiteral("x1"))->value.f64,
            lookup_json_element(pair_elem, stringLiteral("y1"))->value.f64,
        };
        pair_elem = pair_elem->next_sibling;
        pair_count++;
    }
    assert(pair_count <= max_pairs);

    free_json(JSON);

    return pair_count;
}

f64 run_haversine(Pair *haversine_pairs, u32 pair_count) {
    f64 sum = 0;
    for (u32 i = 0; i < pair_count; i++) {
        Pair pair = haversine_pairs[i];
        f64 haversine_result = ReferenceHaversine(pair.x0, pair.y0, pair.x1,
                                                  pair.y1, EARTH_RADIUS);
        sum += haversine_result;
    }

    return sum;
}

#define measureClocks(x)                                                       \
    {                                                                          \
        u64 start_time = read_cpu_timer();                                     \
        x;                                                                     \
        u64 end_time = read_cpu_timer();                                       \
        end_time - start_time                                                  \
    }

int main(int argc, char **argv) {
    u64 cpu_freq = get_cpu_freq(100000);
    u64 total_clocks = read_cpu_timer();

    u64 setup_clocks = read_cpu_timer();
    u64 read_file_clocks = 0;
    u64 misc_setup_clocks = 0;
    u64 parse_json_clocks = 0;
    u64 sum_clocks = 0;
    u64 misc_output_clocks = 0;

    if (argc == 1) {
        printf("Usage:\n"
               "haversine_release [haversine_input.json]\n"
               "haversine_release [haversine_input.json] [answers.f64]\n");
    } else if (argc > 3) {
        printf("Error, wrong number of arguments.\n"
               "Usage:\n"
               "haversine_release [haversine_input.json]\n"
               "haversine_release [haversine_input.json] [answers.f64]\n");
    } else {
        bool check_answers = false;
        if (argc == 3)
            check_answers = true;

        setup_clocks = read_cpu_timer() - setup_clocks;

        read_file_clocks = read_cpu_timer();
        Buffer json_buf = read_entire_file(argv[1]);
        read_file_clocks = read_cpu_timer() - read_file_clocks;

        misc_setup_clocks = read_cpu_timer();
        u32 minimum_json_pair_encoding = 6 * 4;
        u64 max_pair_count = json_buf.count / minimum_json_pair_encoding;
        Pair *haversine_pairs = malloc(sizeof(Pair) * max_pair_count);
        misc_setup_clocks = read_cpu_timer() - misc_setup_clocks;

        parse_json_clocks = read_cpu_timer();
        u32 pair_count =
            parse_haversine(haversine_pairs, &json_buf, max_pair_count);
        parse_json_clocks = read_cpu_timer() - parse_json_clocks;

        f64 sum = 0.0;

        sum_clocks = read_cpu_timer();
        sum = run_haversine(haversine_pairs, pair_count);
        sum_clocks = read_cpu_timer() - sum_clocks;

        misc_output_clocks = read_cpu_timer();
        printf("Input size: %lu\n"
               "Pair count: %d\n"
               "Haversine sum: %.17f\n",
               json_buf.count, pair_count, sum);

        if (check_answers) {
            Buffer ans_buf = read_entire_file(argv[2]);
            f64 *ans_arr = (f64 *)ans_buf.data;
            f64 ref_sum = ans_arr[pair_count];
            printf("\nValidation:\n"
                   "Reference sum: %.17f\n"
                   "Difference: %.17f\n",
                   ref_sum, sum - ref_sum);
        }
        misc_output_clocks = read_cpu_timer() - misc_output_clocks;
    }

    total_clocks = read_cpu_timer() - total_clocks;
    f64 total_time = (f64)total_clocks / (f64)cpu_freq;
    printf("Total time: %f (Cpu freq: %llu)\n", total_time, cpu_freq);
    printf("  Startup: %llu (%.2f%%)\n" 
           "  Read: %llu (%.2f%%)\n" 
           "  Misc Setup: %llu (%.2f%%)\n" 
           "  Parse: %llu (%.2f%%)\n" 
           "  Sum: %llu (%.2f%%)\n" 
           "  Misc Output: %llu (%.2f%%)\n", 
        setup_clocks, 100.0 * (f64)setup_clocks / (f64)total_clocks,
        read_file_clocks, 100.0 * (f64)read_file_clocks / (f64)total_clocks,
        misc_setup_clocks, 100.0 * (f64)misc_setup_clocks / (f64)total_clocks,
        parse_json_clocks, 100.0 * (f64)parse_json_clocks / (f64)total_clocks,
        sum_clocks, 100.0 * (f64)sum_clocks / (f64)total_clocks,
        misc_output_clocks, 100.0 * (f64)misc_output_clocks / (f64)total_clocks);

}
