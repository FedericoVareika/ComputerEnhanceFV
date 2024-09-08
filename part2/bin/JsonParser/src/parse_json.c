#include "parse_json.h"

#define panic(...)                                                             \
    {                                                                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        assert(0);                                                             \
    }

static void set_obj_camp(ObjCamp **parse_camp, Parser *parser);
static void set_arr_camp(ArrCamp **parse_camp, Parser *parser);
static void set_value_node(Value *parse_node, Parser *parser);
static void set_data_node(Data *parse_node, Parser *parser);

// TODO: Make this iterative
static void set_obj_camp(ObjCamp **parse_camp, Parser *parser) {
    assert(parse_camp);

    remove_whitespace(parser);

    char c = parserAt(parser);
    incParser(parser, 1);
    if (c != '}') {
        if (c == ',' || c == '{') {
            *parse_camp = malloc(sizeof(**parse_camp));
            (*parse_camp)->next = NULL;
            (*parse_camp)->camp_data = malloc(sizeof(Data));
            set_data_node((*parse_camp)->camp_data, parser);
            set_obj_camp(&(*parse_camp)->next, parser);
        } else {
            panic("Unexpected obj token: '%c'\n", c);
        }
    }
}

// TODO: Make this iterative
static void set_arr_camp(ArrCamp **parse_camp, Parser *parser) {
    assert(parse_camp);

    remove_whitespace(parser);

    char c = parserAt(parser);
    incParser(parser, 1);
    if (c != ']') {
        if (c == ',' || c == '[') {
            *parse_camp = malloc(sizeof(**parse_camp));
            (*parse_camp)->next = NULL;
            (*parse_camp)->camp_value = malloc(sizeof(Value));
            set_value_node((*parse_camp)->camp_value, parser);
            set_arr_camp(&(*parse_camp)->next, parser);
        } else {
            panic("Unexpected array token: %c\n", c);
        }
    } else {
        *parse_camp = NULL;
    }
}

static void set_data_node(Data *parse_node, Parser *parser) {
    // "key"___ : ___ value
    ValueType key_type = get_value_type(parser);
    assert(key_type == StrVal);
    parse_node->key = parse_string(parser);

    remove_whitespace(parser);

    char c = parserAt(parser);
    assert(c == ':');
    incParser(parser, 1);

    set_value_node(&parse_node->value, parser);
}

static void set_value_node(Value *parse_node, Parser *parser) {
    parse_node->value_type = get_value_type(parser);
    assert(parse_node->value_type != NullVal);
    switch (parse_node->value_type) {
    case NullVal:
    case ValueTypeCount:
    case BoolVal:
        break;
    case StrVal:
        parse_node->data.str = parse_string(parser);
        break;
    case I64Val:
        parse_node->data.i64 = parse_i64(parser);
        break;
    case F64Val:
        parse_node->data.f64 = parse_f64(parser);
        break;
    case ObjVal:
        set_obj_camp(&parse_node->data.obj_camp, parser);
        break;
    case ArrVal:
        set_arr_camp(&parse_node->data.arr_camp, parser);
        break;
    }
}

Value *parse_json(Buffer *buf) {
    assert(buf);
    Parser parser = {buf, 0};
    Value *object = malloc(sizeof(*object));
    set_value_node(object, &parser);
    return object;
}

static void free_json_arr(ArrCamp *arr_camp);
static void free_json_data(Data *data);
static void free_json_value(Value val);
static void free_json_obj(ObjCamp *obj_camp) {
    if (obj_camp) {
        free_json_data(obj_camp->camp_data);
        free_json_obj(obj_camp->next);
        free(obj_camp);
    } 
}

static void free_json_arr(ArrCamp *arr_camp) {
    if (arr_camp) {
        free_json_value(*arr_camp->camp_value);
        free(arr_camp->camp_value);
        free_json_arr(arr_camp->next);
        free(arr_camp);
    }
}

static void free_json_data(Data *data) {
    if (data) {
        free_json_value(data->value);
        free(data->key);
        free(data);
    }
}

static void free_json_value(Value val) {
    switch (val.value_type) {
        case NullVal:
        case I64Val:
        case F64Val:
        case BoolVal:
            break;
        case StrVal: 
            if (val.data.str)
                free(val.data.str);
            break;
        case ObjVal:
            free_json_obj(val.data.obj_camp);
            break;
        case ArrVal:
            free_json_arr(val.data.arr_camp);
            break;
        default:
            break;
    }
}

void free_json(Value *main_obj) {
    if (main_obj) {
        free_json_value(*main_obj);
        free(main_obj);
    }
}

