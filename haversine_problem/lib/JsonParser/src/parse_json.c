#define panic(...)                                                             \
    {                                                                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        assert(0);                                                             \
    }

static void set_json_element(JsonElement *elem, Buffer key, Parser *parser);
static void set_json_list(JsonElement *elem, Parser *parser) {
    JsonElement **next_elem_ptr = &elem->first_son;
    bool has_key = false;
    bool parsing = true;
    while (parsing) {
        remove_whitespace(parser);
        char c = parserAt(parser);
        incParser(parser, 1);
        switch (c) {
        case '{':
            has_key = true;
        case '[': 
        case ',':
            (*next_elem_ptr) = (JsonElement *)malloc(sizeof(JsonElement));
            (*next_elem_ptr)->first_son = NULL; 
            (*next_elem_ptr)->next_sibling = NULL; 
            remove_whitespace(parser);
            Buffer key = {};
            if (has_key) {
                ValueType key_type = get_value_type(parser);
                assert(key_type == StrVal);
                key = parse_buffer(parser);
                (*next_elem_ptr)->key = key;
                remove_whitespace(parser);
                assert(parserAt(parser) == ':'); 
                incParser(parser, 1);
            }
            set_json_element(*next_elem_ptr, key, parser);
            next_elem_ptr = &(*next_elem_ptr)->next_sibling;
            break;
        case '}':
        case ']':
            parsing = false;
            break;
        default: 
            panic("ERROR: Unexpected token '%c'\n", c);
            break;
        }
    }
}

static void set_json_element(JsonElement *elem, Buffer key, Parser *parser) {
    /* timeFunction; */

    elem->key = key;
    remove_whitespace(parser);
    elem->value_type = get_value_type(parser);

    switch (elem->value_type) {
    case NullVal:
    case ValueTypeCount:
    case BoolVal:
        break;
    case StrVal:
        elem->value.str = parse_buffer(parser);
        break;
    case I64Val:
        elem->value.i64 = parse_i64(parser);
        break;
    case F64Val:
        elem->value.f64 = parse_f64(parser);
        break;
    case ObjVal:
        set_json_list(elem, parser);
        break;
    case ArrVal:
        set_json_list(elem, parser);
        break;
    }
    
}

JsonElement parse_json(Buffer *buf) {
    timeFunction; 

    Parser parser = {};
    parser.buf = buf;
    parser.at = 0;
    JsonElement result = {};
    set_json_element(&result, (Buffer){}, &parser);
    return result;
}

JsonElement *lookup_json_element(JsonElement *json, Buffer buf) {
    /* timeFunction; */

    JsonElement *result = json->first_son;
    while (result) {
        if (strncmp(result->key.data, buf.data, buf.count) == 0) break;
        result = result->next_sibling;
    }

    return result;
}

void free_json_element(JsonElement *elem) {
    while (elem) {
        JsonElement *to_free = elem;
        free_json_element(elem->first_son);
        if (elem->value_type == StrVal)
            free(elem->value.str.data);
        elem = elem->next_sibling;
        free(to_free);
    }
}

void free_json(JsonElement main_object) {
    timeFunction;
    free_json_element(main_object.first_son);
}
