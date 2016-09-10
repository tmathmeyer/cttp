#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cref/types.h"
#include "cref/ref.h"
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

HTTP(posthtml) {
    HTTP_STATUS(200, "OK", "text/html");
    HTTP_FILE("post.html");
    HTTP_DONE();
}

HTTP(postimg) {
    HTTP_STATUS(200, "OK", "text/html");
    HTTP_FILE("img.html");
    HTTP_DONE();
}

HTTP(postrecv) {
    HTTP_STATUS(200, "OK", "text/json");
    HTTP_WRITE("{'content': 'data'}");
    HTTP_DONE();
}

HTTP(recieveimages) {
    HTTP_STATUS(200, "OK", "text/json");
    HTTP_WRITE("[");
    multipart_itr(image) {
        if (image->name) {
            puts("writing file!");
            // you should REALLY NOT DO THIS!!!
            // this is an arbitrary write to your filesystem!!!
            int fd = open(image->name, O_RDWR|O_CREAT, 0777);
            write(fd, image->data, image->data_len);
            close(fd);
        }
    }
    HTTP_WRITE("]");
    HTTP_DONE();
}

int main(int argc, char **argv) {
#ifdef DEBUG
    puts("enabling debug mode");
    init_mem_tester();
#endif
    int port = 8088;
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    scoped url_prefix_tree *test = S(_url_prefix_tree(STATIC("")));

    add_to_prefix_tree(test, STATIC("/get/uuid/_var"), &basic);
    add_to_prefix_tree(test, STATIC("/test/file"), &file);
    add_to_prefix_tree(test, STATIC("/api/get/post"), &posthtml);
    add_to_prefix_tree(test, STATIC("/api/img/get"), &postimg);
    add_to_prefix_tree(test, STATIC("/api/img/post"), &recieveimages);
    add_to_prefix_tree(test, STATIC("/api/post/post"), &postrecv);

    http_t *http = create_server(test, port);
    start_http_server(http);
}
