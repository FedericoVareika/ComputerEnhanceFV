#include "all.c"

#define PROFILER 1
#include "platform_metrics.c"
#include "simple_profiler.c"

#include "../lib/JsonParser/src/buffer.c"
#include "../lib/JsonParser/src/structures.c"
#include "../lib/JsonParser/src/parse_values.c"
#include "../lib/JsonParser/src/parse_json.c"

#include "haversine_lib.c"

#define EARTH_RADIUS 6372.8

typedef struct {
    f64 x0, y0;
    f64 x1, y1;
} Pair;

u32 parse_haversine(Pair *haversine_pairs, Buffer *json_buf, u64 max_pairs) {
    timeFunction;

    u32 pair_count = 0;

    JsonElement JSON = parse_json(json_buf);
    JsonElement *pairs = lookup_json_element(&JSON, stringLiteral("pairs"));

    {
        timeBlock("Lookup and Convert");
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
    }

    free_json(JSON);

    return pair_count;
}

f64 run_haversine(Pair *haversine_pairs, u32 pair_count) {
    timeBandwidth(__func__, pair_count * sizeof(Pair));

    f64 sum = 0;
    for (u32 i = 0; i < pair_count; i++) {
        Pair pair = haversine_pairs[i];
        f64 haversine_result = ReferenceHaversine(pair.x0, pair.y0, pair.x1,
                                                  pair.y1, EARTH_RADIUS);
        sum += haversine_result;
    }

    return sum;
}

int main(int argc, char **argv) {
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
        beginProfiler;

        f64 result = -1;

        bool check_answers = false;
        if (argc == 3)
            check_answers = true;

        Buffer json_buf = read_entire_file(argv[1]);

        u32 minimum_json_pair_encoding = 6 * 4;
        u64 max_pair_count = json_buf.count / minimum_json_pair_encoding;
        Pair *haversine_pairs = malloc(sizeof(Pair) * max_pair_count);

        u32 pair_count =
            parse_haversine(haversine_pairs, &json_buf, max_pair_count);

        f64 sum = 0.0;

        sum = run_haversine(haversine_pairs, pair_count);

        printf("Input size: %lu\n"
               "Pair count: %d\n"
               "Haversine sum: %.17f\n",
               json_buf.count, pair_count, sum);

        if (check_answers) {
            Buffer ans_buf = read_entire_file(argv[2]);
            f64 *ans_arr = (f64 *)ans_buf.data;
            f64 ref_sum = ans_arr[pair_count];
            result = sum - ref_sum;
            if (result)
                printf("\nValidation:\n"
                        "Reference sum: %.17f\n"
                        "Difference: %.17f\n",
                        ref_sum, result);
        } 

        end_and_print_profiler();
    }
    return 0;
}

ProfilerEndOfCompilationUnit;
