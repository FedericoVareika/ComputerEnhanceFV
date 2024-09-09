#include "all.c"

/* Value *make_parse_tree() { */
/*     Value *tree = malloc(sizeof(*tree)); */
/*     { */
/*         tree->value_type = ObjVal; */
/*         tree->data.obj_camp = malloc(sizeof(ObjCamp)); */
/*         tree->data.obj_camp->next = NULL; */
/*         tree->data.obj_camp->camp_data = malloc(sizeof(Data)); */

/*         Data *pairs_arr = tree->data.obj_camp->camp_data; */
/*         { */
/*             char *pairs = malloc(strlen("pairs") + 1); */
/*             strcpy(pairs, "pairs"); */
/*             pairs_arr->key = pairs; */
/*             pairs_arr->value.value_type = ArrVal; */
            
/*             pairs_arr->value.data.arr_camp = malloc(sizeof(ArrCamp)); */
/*             ArrCamp *pair1 = pairs_arr->value.data.arr_camp; */
/*             { */
/*                 pair1->camp_value = malloc(sizeof(Value)); */
/*                 pair1->camp_value->value_type = ObjVal; */

/*                 pair1->camp_value->data.obj_camp = malloc(sizeof(ObjCamp)); */
/*                 ObjCamp *pair_camps = pair1->camp_value->data.obj_camp; */
/*                 { */
/*                     pair_camps->camp_data = malloc(sizeof(Data)); */
/*                     char *coord = malloc(strlen("x0") + 1); */
/*                     strcpy(coord, "x0"); */
/*                     pair_camps->camp_data->key = coord; */
/*                     pair_camps->camp_data->value.value_type = F64Val; */
/*                     pair_camps->camp_data->value.data.f64 = 0.11; */
/*                 } */
/*                 pair_camps->next = malloc(sizeof(ObjCamp)); */
/*                 pair_camps = pair_camps->next; */
/*                 { */
/*                     pair_camps->camp_data = malloc(sizeof(Data)); */
/*                     char *coord = malloc(strlen("y0") + 1); */
/*                     strcpy(coord, "y0"); */
/*                     pair_camps->camp_data->key = coord; */
/*                     pair_camps->camp_data->value.value_type = F64Val; */
/*                     pair_camps->camp_data->value.data.f64 = 0.12; */
/*                 } */
/*                 pair_camps->next = malloc(sizeof(ObjCamp)); */
/*                 pair_camps = pair_camps->next; */
/*                 { */
/*                     pair_camps->camp_data = malloc(sizeof(Data)); */
/*                     char *coord = malloc(strlen("x1") + 1); */
/*                     strcpy(coord, "x1"); */
/*                     pair_camps->camp_data->key = coord; */
/*                     pair_camps->camp_data->value.value_type = F64Val; */
/*                     pair_camps->camp_data->value.data.f64 = 0.21; */
/*                 } */
/*                 pair_camps->next = malloc(sizeof(ObjCamp)); */
/*                 pair_camps = pair_camps->next; */
/*                 { */
/*                     pair_camps->camp_data = malloc(sizeof(Data)); */
/*                     char *coord = malloc(strlen("y1") + 1); */
/*                     strcpy(coord, "y1"); */
/*                     pair_camps->camp_data->key = coord; */
/*                     pair_camps->camp_data->value.value_type = F64Val; */
/*                     pair_camps->camp_data->value.data.f64 = 0.22; */
/*                 } */
/*                 pair_camps->next = NULL; */
/*             } */
/*             pair1->next = NULL; */
/*         } */
/*     } */

/*     return tree; */
/* } */

// TODO: Delete parse tree func

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("JsonParse "
                "[--test-pretty-printer] "
                "[--test-parser "
                "file.json]\n");
    } 

    bool test_printer = false;
    bool test_parser = false;
    char *test_json_filename = "data_10_flex.json";
    while (argc > 1) {
        if (strcmp(argv[1], "--test-pretty-printer") == 0)
            test_printer = true;
        else if (strcmp(argv[1], "--test-parser") == 0) {
            test_parser = true;
            test_json_filename = argv[2];
        }
        else 
            break;

        argc--;
        argv++;
    }

    if (test_printer) {
        /* JsonElement *main_obj = make_parse_tree(); */
        /* print_parse_tree(main_obj); */
    }

    if (test_parser) {
        Buffer json_buffer = read_entire_file(test_json_filename);
        JsonElement main_obj = parse_json(&json_buffer);
        print_parse_tree(&main_obj);
    }

    return 0;
}
