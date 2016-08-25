#ifndef _http_syntax_macros_h
#define _http_syntax_macros_h

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

#endif
