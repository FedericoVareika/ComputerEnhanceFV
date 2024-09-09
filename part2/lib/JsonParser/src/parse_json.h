#pragma once

JsonElement parse_json(Buffer *buf);
JsonElement *lookup_json_element(JsonElement *json, Buffer key);
void free_json(JsonElement main_object);
