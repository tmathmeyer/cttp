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
    char *str = "<html><h1>HELLO WORLD:  ";
    char *end = "</h1></html>";
    string *string = api->first;

    write(out, str, strlen(str));
    write(out, string->str, strlen(string->str));
    write(out, end, strlen(end));
    HTTP_DONE();
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
