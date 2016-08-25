
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "http_header_parse.h"
#include "cref/ref.h"
#include "cref/types.h"
#include "cref/list.h"

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
        || c==0
        || c=='\r'
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
            (*location)[index-1] = 0;
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

header_kv *_header_kv(string *key, string *value) {
    header_kv *result = ref_malloc(sizeof(header_kv), 2);
    result->key = S(key);
    result->value = S(value);
    result->destructor = NULL;
    result->hashcode = NULL;
    result->equals = NULL;
    return result;
}

string *copy_until(char *str, char c) {
    size_t i=0;
    while(str[i] && str[i] != c) {
        i++;
    }
    char *dest = calloc(i+1, sizeof(char));
    memcpy(dest, str, i);
    return _string(dest, 1);
}

void free_header(header_t *header) {
    if (!header) {
        return;
    }
    if (header->verb) {
        free(header->verb);
    }
    if (header->path) {
        free(header->path);
    }
    if (header->version) {
        free(header->version);
    }
}

header_t *_header_t() {
    header_t *result = ref_malloc(sizeof(header_t), 1);
    result->verb = NULL;
    result->path = NULL;
    result->version = NULL;
    result->header_keys = EMPTY;
    result->destructor = free_header;
    return result;
}

header_t *stream_parser(stream_t *stream) {
    header_t *header = S(_header_t());
    CONTINUE_IF(skip_spaces(stream, MAX_LOOP));
    CONTINUE_IF(read_until_space(&header->verb, stream, MAX_LOOP));
    CONTINUE_IF(skip_spaces(stream, MAX_LOOP));
    CONTINUE_IF(read_until_space(&header->path, stream, MAX_PATH));
    CONTINUE_IF(skip_spaces(stream, MAX_LOOP));
    CONTINUE_IF(read_until_CRLF(&header->version, stream, MAX_LOOP));

    char *c = NULL;
    list *map = EMPTY;
    while(read_until_CRLF(&c, stream, 512)) {
        if (is_space(c[0])) {
            continue;
        } else {
            string *key = S(copy_until(c, ':'));
            char *val = c+(key->length+1);
            while(is_space(*val)) {
                val++;
            }
            string *value = S(_string(strdup(val), 1));
            header_kv *header = _header_kv(key, value);
            map = _list(header, map);
            L(value);
            L(key);
        }
        free(c);
        c = NULL;
    }
    if (c) free(c);
    header->header_keys = S(map);
    return header;
fail:
    L(header);
    return NULL;
}

