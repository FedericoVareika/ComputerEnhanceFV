#pragma once

void remove_whitespace(FILE *in);
ValueType get_value_type(FILE *in);
char *parse_string(FILE *in);
i64 parse_i64(FILE *in);
f64 parse_f64(FILE *in);
