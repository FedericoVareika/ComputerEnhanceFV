#include "parse_values.h"

#define is_empty_char(a)                                                       \
    (a == ' ' || a == '\t' || a == '\n' || a == '\v' || a == '\f' || a == '\r')

void remove_whitespace(FILE *in) {
    char c = 0;
    fpos_t pos;
    do {
        fgetpos(in, &pos);
        fread(&c, 1, 1, in);
    } while (is_empty_char(c));

    fsetpos(in, &pos);
}

ValueType get_value_type(FILE *in) {
    ValueType type = NullVal;

    char c = 0;
    fpos_t pos;
    do {
        fgetpos(in, &pos);
        fread(&c, 1, 1, in);
    } while (is_empty_char(c));


    if (c == '\"')
        type = StrVal;
    else if ((c >= '0' && c <= '9') || c == '-') {
        do {
            fread(&c, 1, 1, in);
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

    fsetpos(in, &pos);
    return type;
}

#define char_arr_len 100

char *parse_string(FILE *in) {
    char c = 0;
    fread(&c, 1, 1, in);
    assert(c == '\"');

    int i = 0;
    char char_arr[char_arr_len] = "";
    do {
        fread(&c, 1, 1, in);
        char_arr[i++] = c;
    } while (c != '\"' && i < char_arr_len);
    assert(i < char_arr_len);
    char_arr[i - 1] = 0;

    char *str = malloc(sizeof(char) * (i + 1));
    memcpy(str, &char_arr[0], i + 1);
    return str;
}

f64 parse_f64(FILE *in) {
    char c = 0;
    fread(&c, 1, 1, in);
    assert(c == '-' || (c >= '0' && c <= '9'));

    int i = 0;
    fpos_t pos;

    char char_arr[char_arr_len] = "";
    while (c == '-' || c == '.' || (c >= '0' && c <= '9')) {
        char_arr[i++] = c;
        fgetpos(in, &pos);
        fread(&c, 1, 1, in);
    }
    assert(i < char_arr_len);
    char_arr[i] = 0;

    fsetpos(in, &pos);

    f64 float64 = atof(&char_arr[0]);
    return float64;
}

i64 parse_i64(FILE *in) {
    char c = 0;
    fread(&c, 1, 1, in);
    assert(c == '-' || (c >= '0' && c <= '9'));

    int i = 0;
    fpos_t pos;

    char char_arr[char_arr_len] = "";
    while (c == '-' || (c >= '0' && c <= '9')) {
        fread(&c, 1, 1, in);
        fgetpos(in, &pos);
        char_arr[i++] = c;
    }
    assert(i < char_arr_len);
    char_arr[i] = 0;

    fsetpos(in, &pos);

    i64 int64 = atoi(&char_arr[0]);
    return int64;
}
