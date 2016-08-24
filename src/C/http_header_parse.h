#ifndef _http_header_parse_h
#define _http_header_parse_h

#define MAX_LOOP 30
#define MAX_PATH 256
#define BUF_MAX 512

typedef
struct stream_s {
    int fd_read;
    ssize_t bufsz;
    size_t bufindex;
    char buf[BUF_MAX];
    char next;
    bool done;
} stream_t;

typedef
struct header_s {
    char *verb;
    char *path;
    char *version;
}
header_t;

void stream_parser(stream_t *);
stream_t *init_stream(int);

#endif
