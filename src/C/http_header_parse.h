#ifndef _http_header_parse_h
#define _http_header_parse_h

#include "cref/ref.h"
#include "cref/types.h"
#include "cref/list.h"

#define MAX_LOOP 30
#define MAX_PATH 256
#define BUF_MAX 512

typedef
struct stream_s {
    int fd_read;
    ssize_t bufsz;
    size_t bufindex;
    size_t read;
    size_t force_read_size;
    char buf[BUF_MAX];
    char next;
    bool done;
} stream_t;

refstruct(header_t, {
    list *header_keys;
    list *multipart;
    char *verb;
    char *path;
    char *version;
    unsigned short err_code;
});

refstruct(multipart_t, {
    char *name;
    char *type;
    char *data;
    size_t data_len;
});

refstruct(header_kv, {
    string *key;
    string *value;
});

header_t *stream_parser(stream_t *);
stream_t *init_stream(int);
header_kv *_header_kv(string *, string *);
header_t *_header_t();
#endif
