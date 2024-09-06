#include "parse_json.h"

#define panic(...)                                                             \
    {                                                                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        assert(0);                                                             \
    }

/*
 * set_value_node(&main_obj) {
 *      main_obj->value_type = ObjVal
 *      pairs_data = &main_obj->data.obj_camp.camp_data
 *      set_data_node(pairs_data) {
 *          pairs_data->key = "pairs"
 *          set_value_node(pairs_data->value) {
 *              pairs_data->value.value_type = ArrVal;
 *              pair1 = &pairs_data->value.arr_camp
 *
 *              set_arr_camp(pair1) {  [
 *                  camp_value = &pair1->camp_value
 *                  set_node_value(camp_value) {
 *                      camp_value->value_type = ObjVal
 *                      pair1_obj_camp = &camp_value->data
 *                      set_obj_camp(pair1_obj_camp) {
 *                          pair1_obj_camp->camp_data.key = "x0"
 *                          pair1_obj_camp->camp_data.value = 149.743561
 *                          pair1_obj_camp = &pair_obj_camp->next
 *                          set_obj_camp(pair1_obj_camp) {
 *                              pair1_obj_camp->camp_data.key = "y0"
 *                              pair1_obj_camp->camp_data.value = -43.911575
 *                              pair1_obj_camp = &pair_obj_camp->next
 *                              set_obj_camp(pair1_obj_camp) {
 *                                  pair1_obj_camp->camp_data.key = "x1"
 *                                  pair1_obj_camp->camp_data.value = 145.462158
 *                                  pair1_obj_camp = &pair_obj_camp->next
 *                                  set_obj_camp(pair1_obj_camp) {
 *                                      pair1_obj_camp->camp_data.key = "y1"
 *                                      pair1_obj_camp->camp_data.value =
 * -41.396942
 *                                  }
 *                              }
 *                          }
 *                      }
 *                  }
 *                  pair2 = &pair1->next
 *                  set_arr_camp(pair2) {
 *                      ....
 *                      pair3 = &pair2->next
 *                      set_arr_camp(pair3) {
 *                          ...
 *                          pair4 = &pair3->next
 *                          set_arr_camp(pair4) {...}
 *                      }
 *                  }
 *              }
 *          }
 *      }
 * }
 *
 *
 * */
static void set_obj_camp(ObjCamp **parse_camp, FILE *in);
static void set_arr_camp(ArrCamp **parse_camp, FILE *in);
static void set_value_node(Value *parse_node, FILE *in);
static void set_data_node(Data *parse_node, FILE *in);

static void set_obj_camp(ObjCamp **parse_camp, FILE *in) {
    assert(parse_camp);

    remove_whitespace(in);

    char c; 
    fread(&c, 1, 1, in);
    if (c != '}') {
        if (c == ',' || c == '{') {
            *parse_camp = malloc(sizeof(**parse_camp));
            (*parse_camp)->next = NULL;
            (*parse_camp)->camp_data = malloc(sizeof(Data));
            set_data_node((*parse_camp)->camp_data, in);
            set_obj_camp(&(*parse_camp)->next, in);
        } else {
            panic("Unexpected obj token: %c\n", c);
        }
    }
}

static void set_arr_camp(ArrCamp **parse_camp, FILE *in) {
    assert(parse_camp);
    
    remove_whitespace(in);

    char c;
    fread(&c, 1, 1, in);
    if (c != ']') {
        if (c == ',' || c == '[') {
            *parse_camp = malloc(sizeof(**parse_camp));
            (*parse_camp)->next = NULL;
            (*parse_camp)->camp_value = malloc(sizeof(Value));
            set_value_node((*parse_camp)->camp_value, in);
            set_arr_camp(&(*parse_camp)->next, in);
        } else {
            panic("Unexpected array token: %c\n", c);
        }
    }
}

static void set_data_node(Data *parse_node, FILE *in) {
    // "key"___ : ___ value
    ValueType key_type = get_value_type(in);
    assert(key_type == StrVal);
    parse_node->key = parse_string(in);

    remove_whitespace(in);

    char c;
    fread(&c, 1, 1, in);
    assert(c == ':');

    set_value_node(&parse_node->value, in);
}

static void set_value_node(Value *parse_node, FILE *in) {
    parse_node->value_type = get_value_type(in);
    assert(parse_node->value_type != NullVal);
    switch (parse_node->value_type) {
    case NullVal:
    case ValueTypeCount:
    case BoolVal:
        break;
    case StrVal:
        parse_node->data.str = parse_string(in);
        break;
    case I64Val:
        parse_node->data.i64 = parse_i64(in);
        break;
    case F64Val:
        parse_node->data.f64 = parse_f64(in);
        break;
    case ObjVal:
        set_obj_camp(&parse_node->data.obj_camp, in);
        break;
    case ArrVal:
        set_arr_camp(&parse_node->data.arr_camp, in);
        break;
    }
}

Value *parse_json(FILE *in) {
    assert(in);
    Value *object = malloc(sizeof(*object));
    set_value_node(object, in);
    return object;
}
