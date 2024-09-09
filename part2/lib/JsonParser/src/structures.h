#pragma once

typedef enum {
    Tok_error, 

    Tok_open_brace,
    Tok_open_bracket,
    Tok_close_brace,
    Tok_close_bracket,
    Tok_comma,
    Tok_colon,
    Tok_semi_colon,
    Tok_string,
    Tok_number,
    Tok_true,
    Tok_false,
    Tok_null,

    Tok_count,
} JsonToken;

typedef enum {
    NullVal,
    StrVal,
    I64Val,
    F64Val,
    BoolVal,
    ObjVal,
    ArrVal,

    ValueTypeCount,
} ValueType;

typedef union {
    Buffer str;
    i64 i64;
    f64 f64;
} ValueData;

typedef struct JsonElement JsonElement;

struct JsonElement {
    Buffer key;
    ValueType value_type;
    ValueData value;

    JsonElement *first_son;
    JsonElement *next_sibling;
};
