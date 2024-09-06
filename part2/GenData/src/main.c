
void print_arg_format() {
    printf("generate_data "
           "[uniform/cluster] "
           "[seed] "
           "[number of coordinates]\n");
}

void print_error_args(int arg_idx, char **argv) {
    if (arg_idx == -1) {
        printf("Error, argument format is unsupported: \n");
        print_arg_format();
    } else {
        printf("Error, argument ((%s)) is not supported:\n", argv[arg_idx]);
        print_arg_format();
    }
}

void print_gen_config(Method method, u32 seed, u32 n_pairs, f64 expected_sum) {
    char *method_str = "";
    switch (method) {
    case none: 
        method_str = "None";
        break;
    case uniform: 
        method_str = "Uniform";
        break;
    case cluster: 
        method_str = "Cluster";
        break;
    }

    printf("Method: %s\n"
           "Seed: %i\n"
           "Pair count: %i\n" 
           "Expected sum: %f\n", 
           method_str,
           seed,
           n_pairs, 
           expected_sum);
}

int main(int argc, char **argv) {
    int min_arg_count = 3;
    if (argc == 1) {
        print_arg_format();
        return 0;
    } else {
        if (argc != min_arg_count + 1) {
            print_error_args(-1, NULL);
            return 1;
        } else {
            Method method = none;
            if (strcmp(argv[1], "uniform") == 0) {
                method = uniform;
            } else if (strcmp(argv[1], "cluster") == 0) {
                method = cluster;
            } else {
                print_error_args(-1, NULL);
                return 1;
            }

            u32 seed = (u32)atoi(argv[2]);
            u32 n_pairs = (u32)atoi(argv[3]);

            char flex_out_name[50];
            sprintf(flex_out_name, "data_%i_flex.json", n_pairs);
            FILE *flex_out = fopen(flex_out_name, "w");
            if (!flex_out) {
                printf("Error opening flex out file: %s\n", flex_out_name);
                return 1;
            }

            char ans_out_name[50];
            sprintf(ans_out_name, "data_%i_answer.f64", n_pairs);
            FILE *ans_out = fopen(ans_out_name, "w");
            if (!flex_out) {
                printf("Error opening ans out file: %s\n", ans_out_name);
                return 1;
            }

            f64 exp_sum = 
                generate_coords(flex_out, ans_out, method, seed, n_pairs);
            print_gen_config(method, seed, n_pairs, exp_sum);

            fclose(flex_out);
            fclose(ans_out);
        }
    }
}
