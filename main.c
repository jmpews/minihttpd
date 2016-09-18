//
// Created by jmpews on 16/7/11.
//

#include "main.h"
static struct option longopts[] = {
    { "port",       required_argument,      NULL,           'p' },
    { "header",     no_argument,            NULL,           'e' },
    { "body",       no_argument,            NULL,           'b' },
    { "tips",       no_argument,            NULL,           't' },
    { "help",       no_argument,            NULL,           'h' },
    { NULL,         0,                      NULL,           0 }
};

int debug_header = 0, debug_body = 0, debug_tips = 0, port = 8000;
char *root_path = "./htdocs";
char *upload_path = "./uploads";
char *domain = "http://127.0.0.1:8000/download";

int main(int argc, const char *argv[]) {
    ServerInfo *httpd;
    parse_command(argc, argv);
    httpd = startup(&port, root_path, upload_path, domain);
    if (httpd == NULL) {
        printf("ERROR: init socket() error.\n");
        exit(1);
    }
    init_route_handler(httpd);
    printf("> start listening %d.\n", port);

    select_loop(httpd);
}

void parse_command(int argc, const char *argv[]) {
    int opt;
    while((opt = getopt_long_only(argc, (char *const *)argv, "p:ebdh", longopts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'e':
                debug_header = 1;
                break;
            case 'b':
                debug_body= 1;
                break;
            case 't':
                debug_tips = 1;
                break;
            default:
                printf("usage:\n  --port 'listen port'\n  --header 'debug show header'\n  --body 'debug show body'\n  --tips 'debug show tips'\n");
                exit(1);
        }
    }
}

