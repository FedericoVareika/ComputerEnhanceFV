#define EARTH_RADIUS 6372.8

f64 haversine_on_obj(ObjCamp *pair) {
    assert(pair);

    f64 coords[4] = {};
    while (pair) {
        assert(pair->camp_data); 
        int i = 0;
        switch(pair->camp_data->key[0]) {
        case 'x': 
        case 'X': 
            i = 0;
            break;
        case 'y': 
        case 'Y': 
            i = 1;
            break;
        default: 
            fprintf(stderr, "Unexpected object field name: %s\n",
                    pair->camp_data->key);
            return -1;
        }

        switch(pair->camp_data->key[1]) {
        case '0': 
            i += 0;
            break;
        case '1': 
            i += 2;
            break;
        default: 
            fprintf(stderr, "Unexpected object field name: %s\n",
                    pair->camp_data->key);
            return -1;
        }

        coords[i] = pair->camp_data->value.data.f64;
        pair = pair->next;
    }

    return ReferenceHaversine(coords[0], coords[1], coords[2], coords[3],
                              (f64)EARTH_RADIUS);
}

void validate_main_obj(Value *main_obj) {
    assert(main_obj);
    assert(main_obj->value_type == ObjVal);
    assert(main_obj->data.obj_camp);
    assert(main_obj->data.obj_camp->camp_data);
    assert(!strcmp(main_obj->data.obj_camp->camp_data->key, "pairs"));
    assert(main_obj->data.obj_camp->camp_data->value.value_type == ArrVal);
}

void run_haversine(Buffer *json_buf, Buffer *ans_buf, bool check_answers) {
    Value *main_obj = parse_json(json_buf);
    validate_main_obj(main_obj);
    ArrCamp *pairs_arr = 
        main_obj->data.obj_camp->camp_data->value.data.arr_camp;
    assert(pairs_arr);

    f64 *ans_arr = NULL;
    if (check_answers) {
        ans_arr = (f64 *)ans_buf->data;
    }

    int pair_count = 0;
    int incorrect_results = 0;
    f64 sum = 0;

    while (pairs_arr) {
        assert(pairs_arr->camp_value->value_type == ObjVal);

        ObjCamp *pair_obj = pairs_arr->camp_value->data.obj_camp; 
        f64 haversine_result = haversine_on_obj(pair_obj);
        if (check_answers) {
            if (haversine_result != ans_arr[pair_count]) 
                incorrect_results++;
        }
        pair_count++;
        sum += haversine_result;
        pairs_arr = pairs_arr->next;
    }

    printf("Input size: %lu\n"
            "Pair count: %d\n"
            "Haversine sum: %.17f\n",
            json_buf->count, pair_count, sum);

    if (check_answers) {
        f64 ref_sum = ans_arr[pair_count];
        printf("\nValidation:\n"
                "Reference sum: %.17f\n"
                "Difference: %.17f\n"
                "Incorrect results: %d\n",
                ref_sum, sum - ref_sum, incorrect_results);
    }

    free_json(main_obj);
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
        bool check_answers = false;
        if (argc == 3) 
            check_answers = true;

        Buffer json_buf = read_entire_file(argv[1]);

        if (check_answers) {
            Buffer ans_buf = read_entire_file(argv[2]);
            run_haversine(&json_buf, &ans_buf, check_answers);
        } else {
            run_haversine(&json_buf, NULL, check_answers);
        }
    }

}
