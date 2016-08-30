#ifndef _http_syntax_macros_h
#define _http_syntax_macros_h

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>

#define HTTP(name) void name(int out, list *api, header_t *header)
#define HTTP_DONE() \
    do { \
        http_end_write(out); \
        L(header); \
    } while(0)

#define HTTP_STATUS(status, descr, content) \
    do { \
        write(out, "HTTP/1.1 ", 9); \
        write(out, #status, sizeof(#status)); \
        write(out, descr, strlen(descr)); \
        write(out, "\r\n", 2); \
        write(out, "Server: cttp/1.0", 16); \
        write(out, "\r\n", 2); \
        write(out, "Content-Type: ", 14); \
        write(out, content, strlen(content)); \
        write(out, "\r\n", 2); \
        write(out, "\r\n", 2); \
    } while(0)

#define HTTP_WRITE(str) \
    do { \
        write(out, str, strlen(str)); \
    } while(0)

#define HTTP_FILE(path) \
    do { \
        int fd = open(path, O_RDONLY); \
        if (fd > -1) { \
            struct stat fsize; \
            if (!fstat(fd, &fsize)) { \
                sendfile(out, fd, NULL, fsize.st_size); \
            } \
            close(fd); \
        } \
    } while(0)


#endif
