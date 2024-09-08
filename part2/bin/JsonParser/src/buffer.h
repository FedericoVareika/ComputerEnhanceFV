#pragma once

typedef struct {
    size_t count;
    u8 *data;
} Buffer;

typedef struct {
    Buffer *buf;
    u64 at;
} Parser;

#define parserAt(parser) (parser->buf->data[parser->at])
#define parserAtOffset(parser, offset) (parser->buf->data[parser->at + offset])
#define incParser(parser, offset) (parser->at += offset)

Buffer read_entire_file(char *filename);
