#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cref/types.h"
#include "cttp/http.h"
#include "cttp/http_syntax_macros.h"


// the api_handlers are responsible for writing to
// and closing the out sockets!
HTTP(basic) {
    HTTP_STATUS(200, "OK", "text/html");
    string *string = api->first;
    HTTP_WRITE("<html><h1>HELLO WORLD:   ");
    HTTP_WRITE(string->str);
    HTTP_WRITE("</h1></html>");
    HTTP_DONE();
}

HTTP(file) {
    HTTP_STATUS(200, "OK", "text/plain");
    HTTP_FILE("test.txt");
    HTTP_DONE();
}

int main(int argc, char **argv) {
    int port = 8088;
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    scoped url_prefix_tree *test = S(_url_prefix_tree(STATIC("")));
    add_to_prefix_tree(test, STATIC("/get/uuid/_var"), &basic);
    add_to_prefix_tree(test, STATIC("/test/file"), &file);
    http_t *http = create_server(test, port);
    start_http_server(http);
}
