#pragma once

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

typedef struct Data Data;
typedef struct ObjCamp ObjCamp;
typedef struct ArrCamp ArrCamp;

typedef union {
    char *str;
    i64 i64;
    f64 f64;
    ObjCamp *obj_camp;
    ArrCamp *arr_camp;
} ValueData;

typedef struct {
    ValueType value_type;
    ValueData data;
} Value;

struct Data {
    char *key;
    Value value;
};

struct ObjCamp {
    Data *camp_data;
    ObjCamp *next;
};

struct ArrCamp {
    Value *camp_value;
    ArrCamp *next;
};

/* typedef struct JsonElement { */
/*     char *key; */
/*     ValueType value_type; */
/*     ValueData value; */

/*     JsonElement *first_son; */
/*     JsonElement *next_sibling; */
/* } JsonElement; */
