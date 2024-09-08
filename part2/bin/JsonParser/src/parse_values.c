#include "parse_values.h"

#define is_empty_char(a)                                                       \
    (a == ' ' || a == '\t' || a == '\n' || a == '\v' || a == '\f' || a == '\r')

void remove_whitespace(Parser *parser) {
    char c = 0;
    u64 offset = 0;
    do {
        c = parserAtOffset(parser, offset++);
    } while (is_empty_char(c));

    incParser(parser, offset - 1);
}

ValueType get_value_type(Parser *parser) {
    ValueType type = NullVal;


    remove_whitespace(parser);

    u64 offset = 0;
    char c = parserAtOffset(parser, offset++);

    if (c == '\"')
        type = StrVal;
    else if ((c >= '0' && c <= '9') || c == '-') {
        do {
            c = parserAtOffset(parser, offset++);
        } while (c >= '0' && c <= '9');
        if (c == '.')
            type = F64Val;
        else
            type = I64Val;
    } else if (c == 't' || c == 'f')
        type = BoolVal;
    else if (c == '{' || c == '}')
        type = ObjVal;
    else if (c == '[' || c == ']')
        type = ArrVal;

    return type;
}

#define char_arr_len 100

char *parse_string(Parser *parser) {
    u64 offset = 0;
    char c = parserAtOffset(parser, offset++);
    assert(c == '\"');

    int i = 0;
    char char_arr[char_arr_len] = "";
    do {
        c = parserAtOffset(parser, offset++);
        char_arr[i++] = c;
    } while (c != '\"' && i < char_arr_len);
    assert(i < char_arr_len);
    char_arr[i - 1] = 0;

    incParser(parser, offset);

    char *str = malloc(sizeof(char) * (i + 1));
    memcpy(str, &char_arr[0], i + 1);
    return str;
}

f64 parse_f64_sign(Parser *parser) {
    u64 offset = 0;
    char c = parserAtOffset(parser, offset++);

    if (c == '-') {
        return -1.0;
    }

    return 1.0;
}

f64 parse_f64_num(Parser *parser) {
    u64 offset = 0; 
    char c = parserAtOffset(parser, offset++);

    f64 result = 0.0;

    while (1) {
        uint8_t char_num = c - '0';
        if (char_num < 10) {
            result = result * 10.0 + (f64)char_num;
        } else {
            break;
        }
        c = parserAtOffset(parser, offset++);
    }

    incParser(parser, offset - 1);

    return result;
}

f64 parse_f64_(Parser *parser) {
    f64 sign = parse_f64_sign(parser);
    f64 result = parse_f64_num(parser);
    f64 exp = 1.0 * 10.0;

    u64 offset = 0;

    char c = parserAtOffset(parser, offset++);

    if (c == '.') {
        while (1) {
            c = parserAtOffset(parser, offset++);
            uint8_t char_num = c - '0';
            if (char_num < 10) {
                result = result + ((f64)char_num / exp);
                exp = exp * 10.0;
            } else {
                break;
            }
        }
        incParser(parser, offset - 1);
    }

    result = result * sign;
    return result;
}

f64 parse_f64(Parser *parser) {
    u64 offset = 0;
    char c = parserAtOffset(parser, offset++);
    assert(c == '-' || (c >= '0' && c <= '9'));

    int i = 0;

    char char_arr[char_arr_len] = "";
    while (c == '-' || c == '.' || (c >= '0' && c <= '9')) {
        char_arr[i++] = c;
        c = parserAtOffset(parser, offset++);
    }
    assert(i < char_arr_len);
    char_arr[i] = 0;

    incParser(parser, offset - 1);

    f64 float64 = atof(&char_arr[0]);
    return float64;
}

i64 parse_i64(Parser *parser) {
    u64 offset = 0;
    char c = parserAtOffset(parser, offset++);
    assert(c == '-' || (c >= '0' && c <= '9'));

    int i = 0;

    char char_arr[char_arr_len] = "";
    while (c == '-' || (c >= '0' && c <= '9')) {
        c = parserAtOffset(parser, offset++);
        char_arr[i++] = c;
    }
    assert(i < char_arr_len);
    char_arr[i] = 0;

    incParser(parser, offset);

    i64 int64 = atoi(&char_arr[0]);
    return int64;
}
