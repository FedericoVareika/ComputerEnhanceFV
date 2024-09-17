typedef struct {
    size_t count;
    u8 *data;
} Buffer;

typedef struct {
    Buffer *buf;
    u64 at;
} Parser;

#define stringLiteral(string) ((Buffer){sizeof((string)) - 1, (u8 *)(string)})

#define parserAt(parser) (parser->buf->data[parser->at])
#define parserAtOffset(parser, offset) (parser->buf->data[parser->at + offset])
#define incParser(parser, offset) (parser->at += offset)

static Buffer allocateBuffer(size_t count) {
    Buffer result = {};
    result.data = malloc(sizeof(*result.data) * count);
    if (result.data) {
        result.count = count;
    } else {
        fprintf(stderr, "Could not allocate buffer of size: %d\n", (u32)count);
    }

    return result;
}

void free_buffer(Buffer *buffer) {
    if (buffer->data)
        free(buffer->data);
}

Buffer read_entire_file(char *filename) {
    Buffer result = {0, 0};

    FILE *in = fopen(filename, "r");
    if (in) {
#if _WIN32
        struct __stat64 stat;
        _stat64(filename, &stat);
#else
        struct stat stat_;
        stat(filename, &stat_);
#endif

        result = allocateBuffer(stat_.st_size);
        if (result.data) {
            timeBandwidth("fread", result.count);
            if (fread(result.data, result.count, 1, in) != 1) {
                fprintf(stderr, "Error: Unable to read \"%s\".\n", filename);
                free_buffer(&result);
            }
        }

        fclose(in);
    } else {
        fprintf(stderr, "Error: Unable to open \"%s\".\n", filename);
    }

    return result;
}
