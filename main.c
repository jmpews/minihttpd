//
// Created by jmpews on 16/7/11.
//
#include "main.h"
#include "utils.h"
#include <libgen.h>

#include <getopt.h>
static struct option longopts[] = {
    { "port",       required_argument,      NULL,           'p' },
    { "header",     no_argument,            NULL,           'e' },
    { "body",       no_argument,            NULL,           'b' },
    { "tips",       no_argument,            NULL,           't' },
    { "help",       no_argument,            NULL,           'h' },
    { NULL,         0,                      NULL,           0 }
};


void select_loop(ServerInfo *httpd);

int main(int argc, const char *argv[]) {

    int opt;
    int port = 8000;
    ServerInfo *httpd;

    while((opt = getopt_long_only(argc, argv, "p:ebdh", longopts, NULL)) != -1) {
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
                printf("usage:\n--port 'listen port'\n--header 'debug show header'\n--body 'debug show body'\n--tips 'debug show tips'\n");
        }
    }
    httpd = startup(&port);
    if (httpd == NULL) {
        printf("ERROR: init socket() error.\n");
        exit(1);
    }

    init_route(httpd);
    printf("> start listening at %d\n", port);

    select_loop(httpd);
}


//*******************************************  Select模块  ********************************
void select_loop(ServerInfo *httpd) {
    struct sockaddr_in client_addr;
    struct timeval tv;
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr_in);

    char c;
    int i, j, r, MAX_CLIENTS;
    MAX_CLIENTS = 1024;

    int client_fd;
    int client_array[MAX_CLIENTS];
    int server_fd = httpd->fd;
    SocketNode *client_sock=NULL;
    memset(client_array, 0, MAX_CLIENTS * sizeof(int));
    int maxfd;
    maxfd = 0;

    fd_set read_fds;
    fd_set write_fds;
    fd_set exception_fds;
    fd_set tmp_read_fds;
    fd_set tmp_write_fds;
    fd_set tmp_exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&exception_fds);

    char ipaddr[32];

    FD_SET(server_fd, &read_fds);
    client_array[server_fd] = 1;

    while (1) {
        tmp_read_fds = read_fds;
        tmp_write_fds = write_fds;
        tmp_exception_fds = exception_fds;

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_array[i]) if (maxfd < i)
                maxfd = i;
        }
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        r = select(maxfd + 2, &tmp_read_fds, &tmp_write_fds, &tmp_exception_fds, &tv);
        if (r < 0)
            perror("ERROR: select() error.");
        else {
            if (FD_ISSET(server_fd, &tmp_read_fds)) {
                client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &addr_len);
                if (client_fd < 0)
                    printf("ERROR :client connect error.\n");
                else {
                    inet_ntop(AF_INET, &(client_addr.sin_addr), ipaddr, 32 * sizeof(char));
                    printf("> [socket-%d] accept, detail{ip: %s}\n", client_fd, ipaddr);
                    set_nonblocking(client_fd);
                    SocketNode *tmp = new_socket_node(client_fd);
                    tmp->client_fd = client_fd;
                    add_socket_node(httpd->head_node, tmp);

                    FD_SET(client_fd, &read_fds);
                    client_array[client_fd] = 1;
                    maxfd = (maxfd < client_fd) ? client_fd : maxfd;
                }
            }
            else {
                for (i = server_fd + 1; i <= maxfd; i++) {
                    if (client_array[i]) {
                        if (FD_ISSET(i, &tmp_read_fds)) {
                            r = recv(i, &c, 1, MSG_PEEK);
                            if (r < 1) {
                                printf("DEBUG: close [fd-%d]\n", i);
                                free_socket_node(httpd->head_node, i);
                                FD_CLR(i, &read_fds);
                                client_array[i] = 0;
                                fflush(stdout);
                                continue;
                            }
                            //read data or close connect
                            client_sock=find_socket_node(httpd->head_node,i);
                            r = handle_request(client_sock);
                            if (r == IO_DONE) {
                                FD_CLR(i, &read_fds);
                                FD_SET(i, &write_fds);
                            } else if (r == IO_ERROR) {
                                printf("DEBUG: close [fd-%d]\n", i);
                                FD_CLR(i, &read_fds);
                                free_socket_node(httpd->head_node, i);
                                client_array[i] = 0;
                            }
                            else if (r == IO_EAGAIN) {
                                printf("EAGAIN:wow.");
                            }
                        }
                        else if (FD_ISSET(i, &tmp_write_fds)) {
                            //send data
                            client_sock=find_socket_node(httpd->head_node,i);
                            r = handle_response(client_sock, httpd);
                            if (r == IO_DONE || r == IO_ERROR) {
                                shutdown(i, SHUT_WR);
                                FD_CLR(i, &write_fds);
                                FD_SET(i, &read_fds);
                            }
                        }
                    }
                }
            }
        }
    }
}

//*******************************************  EPoll模块  ********************************

#ifdef DEF_EPOLL

#include <sys/epoll.h>
void epoll_loop(ServerInfo *httpd) {
    INT_32 epoll_fd, nfds;
    int buffer_size;
    int MAX_CLIENTS=1024;
    INT_32 client_fd;
    INT_32 s;
    INT_32 i;
    INT_32 r;
    char c;
    struct epoll_event ev;
    struct epoll_event *events;
    struct sockaddr_in client_addr;
    char ipaddr[32];
    int server_fd = httpd->fd;
    SocketNode *head_node = httpd->head_node;

    socklen_t socket_len = sizeof(struct sockaddr_in);

    epoll_fd = epoll_create(MAX_CLIENTS);
    ev.data.fd = server_fd;
    ev.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    if (s == -1) {
        printf("! epoll_ctl error.\n");
        abort();
    }
    events = (struct epoll_event *) malloc(MAX_CLIENTS * sizeof(struct epoll_event));
    if (events == NULL) {
        printf("! malloc error.\n");
    }
    memset(events, 0, MAX_CLIENTS * sizeof(struct epoll_event));
    for (; ;) {
        nfds = epoll_wait(epoll_fd, events, MAX_CLIENTS, 3000);
        for (i = 0; i < nfds; i++) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                printf("! epoll error.\n");
                //abort();
                continue;
            }
            else if (events[i].events & EPOLLIN) {

                if (events[i].data.fd == server_fd) {
                    //ET until get the errno=EAGAIN
                    while ((client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &socket_len)) > 0) {
                        inet_ntop(AF_INET, &(client_addr.sin_addr), ipaddr, 32 * sizeof(char));
                        printf("> SOCKET[%d] Accept : %s\n", client_fd, ipaddr);
                        set_nonblocking(client_fd);
                        ev.data.fd = client_fd;
                        ev.events = EPOLLIN | EPOLLET;
                        s = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                        SocketNode *client_sock = new_socket_node();
                        client_sock->client_fd = client_fd;
                        add_socket_node(SocketHead,client_sock);
                    }
                    if (errno == EAGAIN) TIP printf("! accept EAGAIN\n");
                    //printf("! EAGAIN try again\n");
                    else
                        perror("! Accept Error:");
                    continue;
                }
                else {
                    // 判断读取到0长度数据表明，客户端主动关闭
                    r = recv(i, &c, 1, MSG_PEEK);
                    if(r<1){
                        printf("CLOSE=ID:%d\n",i);
                        free_socket_node(SocketHead, i);
                        FD_CLR(i,&read_fds);
                        client_array[i]=0;
                        fflush(stdout);
                        continue;
                    }
                    r = handle_request(events[i].data.fd);
                    if (r == IO_DONE) {
                        ev.data.fd = events[i].data.fd;
                        ev.events = EPOLLOUT | EPOLLET;
                        s = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                    }else if (r==IO_ERROR) {
                        printf("IO_ERROR=ID:%d",i);
                        free_socket_node(SocketHead, i);
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
                            printf("! close epoll_ctl error.\n");
                    }
                    else if(r==IO_EAGAIN){
                        TIP printf("EAGAIN:wow.");
                    }

                }
            }
            else if (events[i].events & EPOLLOUT) {
                r = handle_response(events[i].data.fd);
                if(r==IO_DONE||r==IO_ERROR){
                    //优雅关闭socekt，采用shutdown，先关闭写通道，然后让客户端主动关闭
                    shutdown(i,SHUT_WR);
                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                }
            }
            else {
                printf("uncatched.");
            }
        }
    }
}
#endif