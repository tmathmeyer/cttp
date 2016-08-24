
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "http_header_parse.h"

#define CONTINUE_IF(fn) \
    do { \
        if (!(fn)) { \
            goto fail; \
        } \
    } while(0)

bool _get_next_char(stream_t *stream, char *write) {
    if (stream->bufsz == 0) {
        return false;
    }
    if (stream->bufsz > BUF_MAX) {
        return false;
    }
    if (stream->bufindex < stream->bufsz) {
        *write = (stream->buf)[stream->bufindex++];
        return true;
    }
    if (stream->bufindex == stream->bufsz) {
        *write = (stream->buf)[stream->bufindex];
        stream->bufindex = 0;
        if (stream->bufsz == BUF_MAX) {
            stream->bufsz = read(stream->fd_read, stream->buf, BUF_MAX);
            return true;
        }
        stream->bufsz = 0;
        return true;
    }
}

bool peek_char(stream_t *stream, char *write) {
    if (stream->done) {
        return false;
    }
    *write = stream->next;
    return true;
}

bool pop_char(stream_t *stream, char *write) {
    if (stream->done) {
        return false;
    }
    *write = stream->next;

    if (!_get_next_char(stream, &stream->next)) {
        stream->done = true;
    }
    return true;
}

stream_t *init_stream(int fd) {
    stream_t *result = calloc(1, sizeof(stream_t));
    result->fd_read = fd;
    result->bufindex = 0;
    result->bufsz = read(fd, result->buf, BUF_MAX);
    char bogus;
    pop_char(result, &bogus);
    return result;
}

bool is_space(char c) {
    return c==' '
        || c=='\t'
        || c=='\n';
}

bool skip_spaces(stream_t *stream, size_t max) {
    char test;
    while(max--) {
        if(peek_char(stream, &test)) { if (is_space(test)) {
                pop_char(stream, &test); // if space, pop to next
            } else {
                return true;
            }
        } else {
            return false;
        }
    }
}

bool read_until_space(char **location, stream_t *stream, size_t max) {
    bool alloc = *location==NULL; // we are responsible for allocation or not
    size_t index = 0;
    size_t strsize = 10;
    if (alloc) {
        *location = calloc(strsize, sizeof(char));
    }
    while(max--) {
        char tmp;
        if (!peek_char(stream, &tmp)) {
            return index > 0;
        }
        if (is_space(tmp)) {
            return true;
        }
        (*location)[index] = tmp;
        pop_char(stream, &tmp);
        index++;
        if (index+1 == strsize) {
            strsize *=2;
            *location = realloc(*location, strsize*sizeof(char));
        }
    }
    return true;
}

bool read_until_CRLF(char **location, stream_t *stream, size_t max) {
    bool alloc = *location==NULL; // we are responsible for allocation or not
    bool crlf = false;
    char junk;
    size_t index = 0;
    size_t strsize = 10;
    if (alloc) {
        *location = calloc(strsize, sizeof(char));
    }
    while(max--) {
        char tmp;
        if (!peek_char(stream, &tmp)) {
            return index > 0;
        }
        if (tmp=='\n' && crlf) {
            (*location)[index] = 0;
            pop_char(stream, &junk);
            return true;
        }
        if (tmp=='\r') {
            crlf=true;
        } else {
            crlf=false;
        }
        (*location)[index] = tmp;
        pop_char(stream, &junk);
        index++;
        if (index+1 == strsize) {
            strsize *=2;
            *location = realloc(*location, strsize*sizeof(char));
        }
    }
    return true;
}

void stream_parser(stream_t *stream) {
    header_t *header = calloc(1, sizeof(header_t));
    CONTINUE_IF(skip_spaces(stream, MAX_LOOP));
    CONTINUE_IF(read_until_space(&header->verb, stream, MAX_LOOP));
    CONTINUE_IF(skip_spaces(stream, MAX_LOOP));
    CONTINUE_IF(read_until_space(&header->path, stream, MAX_PATH));
    CONTINUE_IF(skip_spaces(stream, MAX_LOOP));
    CONTINUE_IF(read_until_CRLF(&header->version, stream, MAX_LOOP));

    puts(header->verb);
    puts(header->path);
    puts(header->version);


    free(header->verb);
    free(header->path);
    free(header->version);

    char *c = NULL;
    int ct = 0;
    while(read_until_CRLF(&c, stream, 512)) {
        printf("%s\n", c);
    }
    puts("=========================");
    if (stream->done) {
        puts("SUCCESS");
    }

fail:
    free(header);
}
