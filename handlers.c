#include "typedata.h"
#include "utils.h"
#include "main.h"

extern int send_file(int client_fd, char *path, long *start);
extern void send_404(int client_fd);
extern void send_data(int client, char *data);

int default_handler(SocketNode *client_sock, ServerInfo *httpd) {
    char response_path[256];
    struct stat st;
    int r;
    sprintf(response_path, "%s/htdocs%s", httpd->rootpath, client_sock->request.request_path);
    if (response_path[strlen(response_path) - 1] == '/')
        strcat(response_path, "index.html");
    response_path[strlen(response_path)] = '\0';
    if ((stat(response_path, &st) != -1) && ((st.st_mode & S_IFMT) == S_IFREG)) {
         r = send_file(client_sock->client_fd, response_path, &client_sock->response.response_cache_len);
         if(r == IO_DONE_W) {
             return IO_DONE_W;
         } else if(r == IO_EAGAIN_W) {
             return IO_EAGAIN_W;
         }
    } else {
        send_404(client_sock->client_fd);
        return IO_DONE_W;
    }
    return IO_DONE_W;
}



int echo_handler(SocketNode *client_sock, ServerInfo *httpd) {
    send_data(client_sock->client_fd, client_sock->request.header_dump);
    return IO_DONE_W;
}

void init_route_handler(ServerInfo *httpd) {
    ListNode *head_route = new_list_node();
    head_route->data = (void *)new_route_handler("*", default_handler, R_BODY);
    head_route->next = NULL;
    httpd->head_route = head_route;
    list_append(httpd->head_route, (void *)new_route_handler("/echo", echo_handler, R_BODY));
}
