/* static void print_obj_camp(ObjCamp *camp, int depth); */
/* static void print_arr_camp(ArrCamp *camp, int depth); */
/* static void print_value_node(Value *value, int depth); */
/* static void print_data_node(Data *data, int depth); */

/* static void print_obj_camp(ObjCamp *camp, int depth) { */
/*     if (camp) { */
/*         print_data_node(camp->camp_data, depth + 1); */
/*         if (camp->next) { */
/*             printf(",\n"); */
/*             print_obj_camp(camp->next, depth); */
/*         } else { */
/*             printf("\n"); */
/*         } */
/*     } */
/* } */

/* static void print_arr_camp(ArrCamp *camp, int depth) { */
/*     if (camp) { */
/*         print_value_node(camp->camp_value, depth + 1); */
/*         if (camp->next) { */
/*             printf(", "); */
/*             print_arr_camp(camp->next, depth); */
/*         } else { */
/*             printf("\n"); */
/*         } */
/*     } */
/* } */

/* static void print_value_node(Value *value, int depth) { */
/*     switch (value->value_type) { */
/*     case NullVal: */
/*     case ValueTypeCount: */
/*     case BoolVal: */
/*         printf("not supported"); */
/*         break; */
/*     case StrVal: */
/*         assert(value->data.str); */
/*         printf("%s", value->data.str); */
/*         break; */
/*     case I64Val: */
/*         printf("%ld", value->data.i64); */
/*         break; */
/*     case F64Val: */
/*         printf("%.17f", value->data.f64); */
/*         break; */
/*     case ObjVal: */ 
/*         printf("\n%*s", depth + 1, ""); */
/*         printf("{\n"); */
/*         print_obj_camp(value->data.obj_camp, depth + 1); */
/*         printf("%*s", depth + 1, ""); */
/*         printf("}"); */
/*         break; */
/*     case ArrVal: */ 
/*         printf("\n%*s", depth + 1, ""); */
/*         printf("["); */
/*         print_arr_camp(value->data.arr_camp, depth + 1); */
/*         printf("%*s", depth + 1, ""); */
/*         printf("]"); */
/*         break; */
/*     } */
/* } */

/* static void print_data_node(Data *data, int depth) { */
/*     if (!data) */
/*         return; */
/*     printf("%*s", depth, ""); */
/*     printf("%s : ", data->key); */
/*     print_value_node(&data->value, depth); */
/* } */

/* void print_parse_tree(Value *main_object) { */
/*     print_value_node(main_object, 0); */
/*     printf("\n"); */
/* } */

void print_json_elem(JsonElement *elem, int depth);
void print_json_list(JsonElement *elem, int depth) {
    while (elem) {
        print_json_elem(elem, depth + 1);
        if (elem->next_sibling) 
            printf(",");
        printf("\n");
        elem = elem->next_sibling;
    }
}

void print_json_elem(JsonElement *elem, int depth) {
    printf("%*s", depth, "");

    if (elem->key.count) {
        printf("\"%.*s\": ", (int)elem->key.count, elem->key.data);
    }

    switch (elem->value_type) {
    case NullVal:
    case ValueTypeCount:
    case BoolVal:
        printf("not supported");
        break;
    case StrVal:
        assert(elem->value.str.data);
        printf("%s", elem->value.str.data);
        break;
    case I64Val:
        printf("%ld", elem->value.i64);
        break;
    case F64Val:
        printf("%.17f", elem->value.f64);
        break;
    case ObjVal: 
        printf("\n%*s", depth + 1, "");
        printf("{\n");
        print_json_list(elem->first_son, depth + 1);
        printf("%*s}", depth + 1, "");
        break;
    case ArrVal: 
        printf("\n%*s", depth + 1, "");
        printf("[");
        print_json_list(elem->first_son, depth + 1);
        printf("%*s]", depth + 1, "");
        break;
    }

}

void print_parse_tree(JsonElement *main_object) {
    print_json_elem(main_object, 0);
    printf("\n");
}
