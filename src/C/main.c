#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cref/types.h"
#include "http.h"

// the api_handlers are responsible for writing to
// and closing the out sockets!
void basic(int out, list *api_parts, header_t *header) {
    char *str = "HTTP/1.1 200 OK\r\n" \
                "Server: cttp/1.0\r\n" \
                "Content-Type: text/html\r\n" \
                "\r\n" \
                "<html><h1>HELLO WORLD:  ";
    char *end = "</h1></html>";
    string *string = api_parts->first;

    write(out, str, strlen(str));
    write(out, string->str, strlen(string->str));
    write(out, end, strlen(end));
    
    http_end_write(out);
    L(header);
}

int main(int argc, char **argv) {
    int port = 8088;
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    scoped url_prefix_tree *test = S(_url_prefix_tree(STATIC("")));
    add_to_prefix_tree(test, STATIC("/get/uuid/_var"), &basic);
    http_t *http = create_server(test, port);
    start_http_server(http);
}
