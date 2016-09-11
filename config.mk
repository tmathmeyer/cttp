exported := C/http.h C/http_header_parse.h C/http_syntax_macros.h C/thread_pool.h
project := cttp
link_libs := -lpthread -rdynamic
CFLAGS := -g -rdynamic

