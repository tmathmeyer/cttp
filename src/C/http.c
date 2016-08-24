#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <linux/sockios.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "cref/ref.h"
#include "cref/types.h"
#include "cref/list.h"

#include "http.h"
#include "http_header_parse.h"

void *server_run(void *data) {
    http_t *http = data;
    int client_sock;
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    listen(http->_socket, 5);
    clilen = sizeof(cli_addr);

    while(http->running) {
        client_sock = accept(http->_socket, (struct sockaddr *)&cli_addr, &clilen);
        if (client_sock < 0) {
            perror("cant accept socket");
            exit(1);
        }
        char dump[256] = {0};
        int i;

        stream_t *stream = init_stream(client_sock);
        stream_parser(stream);



        list *variables = EMPTY;
        url_prefix_tree *get = lookup(http->urls, STATIC("/get/uuid/12345"), &variables);
        if (get->handler) {
            (get->handler)(client_sock, variables);
        }
        L(variables);
        L(get);
    }
}

void start_http_server(http_t *http) {
    http->running = true;
    pthread_create(&(http->_server_thread), NULL, server_run, http);
    pthread_join(http->_server_thread, NULL);
}

http_t *create_server(url_prefix_tree *tree, int port) {
    http_t *http = calloc(sizeof(http_t), 1);
    http->_port = port;
    http->urls = tree;
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        free(http);
        goto fail;
    }
    http->_socket = server_socket;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) {
        free(http);
        //TODO cleanup the socket?!?!
        goto fail;
    }
    return http;

fail:
    perror("FAILED :(\n");
    exit(1);
    return NULL;
}

url_prefix_tree *_url_prefix_tree(string *prefix) {
    url_prefix_tree *result = ref_malloc(sizeof(url_prefix_tree), 3);
    result->destructor = NULL;
    result->hashcode = NULL;
    result->equals = NULL;
    result->prefix = S(prefix);
    result->children = EMPTY;
    result->_var = NULL;
    result->handler = NULL;
    return result;
}

list *reverse(list *ls) {
    S(ls);
    list *res = EMPTY;
    list *free = ls;
    while(ls != EMPTY) {
        res = _list(ls->first, res);
        ls = ls->rest;
    }
    L(free);
    return S(res);
}

list *split(string *str, char *tok) {
    char *s = strdup(str->str);
    char *f = s;
    char *e = NULL;
    list *r = EMPTY;
    while(e = strstr(s, tok)) {
        char t = e[0];
        e[0] = 0;
        r = _list(_string(strdup(s), 1), r);
        e[0] = t;
        s = e+strlen(tok);
    }
    r = _list(_string(strdup(s), 1), r);
    free(f);
    return reverse(r);
}

url_prefix_tree *get_by_name(string *name, list *of) {
    while(of != EMPTY) {
        url_prefix_tree *upt = of->first;
        if (string_equals(name, upt->prefix)) {
            return upt;
        }
        of = of->rest;
    }
    return NULL;
}

void _recurse_add(url_prefix_tree *tree, list *url_parts, void (*h)(int, list *)) {
    if (url_parts == EMPTY) {
        tree->handler = h;
        return;
    }

    if (url_parts->first == NULL) {
        return;
    }

    if (strlen(((string *)(url_parts->first))->str) == 0) {
        _recurse_add(tree, url_parts->rest, h);
        return;
    }

    string *grp = url_parts->first;
    if (string_equals(STATIC("_var"), grp)) {
        if (tree->_var == NULL) {
            tree->_var = S(_url_prefix_tree(STATIC("_var")));
        }
        _recurse_add(tree->_var, url_parts->rest, h);
        return;
    }

    url_prefix_tree *new = get_by_name(url_parts->first, tree->children);
    if (new == NULL) {
        new = _url_prefix_tree(url_parts->first);
        tree->children = S(_list(new, tree->children));
        L(tree->children->rest);
    }
    _recurse_add(new, url_parts->rest, h);
}

void add_to_prefix_tree(url_prefix_tree *tree, string *url, void (*hand)(int, list *)) {
    S(url);
    S(tree);
    scoped list *parts = split(url, "/");
    _recurse_add(tree, parts, hand);
    L(url);
    L(tree);
}

void print_tree(url_prefix_tree *tree, int indent) {
    indent++;
    S(tree);
    puts(tree->prefix->str);
    if (tree->_var != NULL) {
        for(int i=0;i<indent;i++) printf("    ");
        printf("|--");
        print_tree(tree->_var, indent);
    }
    list *ch = tree->children;
    while(ch != EMPTY) {
        url_prefix_tree *n = ch->first;
        for(int i=0;i<indent;i++) printf("    ");
        printf("|--");
        print_tree(n, indent);
        ch = ch->rest;
    }
    L(tree);
}

url_prefix_tree *_lookup(url_prefix_tree *upt, list *parts, list **vars) {
    if (parts == EMPTY) {
        return upt;
    }

    if (parts->first == NULL) {
        return NULL;
    }

    if (strlen(((string *)(parts->first))->str) == 0) {
        return _lookup(upt, parts->rest, vars);
    }

    string *first = parts->first;
    url_prefix_tree *look = get_by_name(first, upt->children);
    if (look == NULL) {
        look = upt->_var;
        if (look == NULL) {
            return NULL;
        } else if (vars != NULL) {
            list *_k = _list(first, *vars);
            *vars = _k;
        }
    }

    return _lookup(look, parts->rest, vars);
}

url_prefix_tree *lookup(url_prefix_tree *upt, string *str, list **vars) {
    S(upt); S(str);
    scoped list *parts = split(str, "/");
    url_prefix_tree *result = S(_lookup(upt, parts, vars));
    S(*vars);
    list *k = reverse(*vars);
    L(*vars);
    *vars = k;
    L(upt); L(str);
    return result;
}

void http_end_write(int socket) {
    int pending = 0;
    while(ioctl(socket, SIOCOUTQ, &pending),pending);
    close(socket);
    wait(NULL);
}

