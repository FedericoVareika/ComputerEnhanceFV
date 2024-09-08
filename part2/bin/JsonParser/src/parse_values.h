#pragma once

void remove_whitespace(Parser *parser);
ValueType get_value_type(Parser *parser);
char *parse_string(Parser *parser);
i64 parse_i64(Parser *parser);
f64 parse_f64(Parser *parser);
