#pragma once

Value *parse_json(Buffer *buf);
void free_json(Value *main_obj);
