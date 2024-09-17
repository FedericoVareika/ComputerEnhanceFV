#include "all.c"

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
