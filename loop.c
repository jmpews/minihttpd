//
//  loop.c
//  jmp2httpd
//
//  Created by jmpews on 16/7/31.
//  Copyright © 2016年 jmpews. All rights reserved.
//

#include "loop.h"
#include "utils.h"
#include <arpa/inet.h>

int register_handler(LP *loop, int fd, void *watcher_cb, int stat, ServerInfo *httpd) {
    SocketNode *client_sock = find_socket_node(httpd->head_node, fd);
    watcher_add(client_sock, watcher_cb, stat);
    client_sock->events |= stat;
    return 0;
}
void select_loop(ServerInfo *httpd) {
    int (*func)(SocketNode *, ServerInfo *);
    watcher *tmp;
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
            perror("ERROR-[select_loop]: select() error.\n");
        else {
            if (FD_ISSET(server_fd, &tmp_read_fds)) {
                client_fd = accept_handler(httpd);
                register_handler(NULL, client_fd, (void *)handle_request, IO_READ, httpd);
                FD_SET(client_fd, &read_fds);
                client_array[client_fd] = 1;
                maxfd = (maxfd < client_fd) ? client_fd : maxfd;
            }
            else {
                for (i = server_fd + 1; i <= maxfd; i++) {
                    if (client_array[i]) {
                        if (FD_ISSET(i, &tmp_read_fds)) {
                            r = recv(i, &c, 1, MSG_PEEK);
                            if (r < 1) {
                                printf("> [socket-%d] closed.\n", i);
                                free_socket_node(httpd->head_node, i);
                                FD_CLR(i, &read_fds);
                                client_array[i] = 0;
                                continue;
                            }
                            client_sock=find_socket_node(httpd->head_node,i);
                            tmp = client_sock->w;
                            while(tmp) {
                                if(tmp->events&IO_READ) {
                                    func = tmp->watcher_cb;
                                    r = func(client_sock, httpd);
                                
                            
                                    //if(client_sock->handler != NULL)
                                    //    r = handle_response_with_handler(client_sock, httpd);
                                    //else
                                    //    r = handle_request(client_sock, httpd);
                                    if (r == IO_DONE_R) {
                                        FD_CLR(i, &read_fds);
                                        FD_SET(i, &write_fds);
                                    }
                                    else if (r == IO_DONE_W) {
                                        shutdown(i, SHUT_WR); //不用清除状态 因为下次关闭时读取0字节
                                    }
                                    else if (r == IO_ERROR) {
                                        printf("> [socket-%d] error closed.\n", i);
                                        FD_CLR(i, &read_fds);
                                        free_socket_node(httpd->head_node, i);
                                        client_array[i] = 0;
                                    }
                                    else if (r == IO_EAGAIN_R) {
                                        printf("WARNNING: read EAGAIN code.\n");
                                    }
                                    else if (r == IO_EAGAIN_W) {
                                        FD_CLR(i, &read_fds);
                                        FD_SET(i, &write_fds); //重新加入select中
                                        printf("WARNNING: write EAGAIN code.\n");
                                    }
                                }
                                tmp = tmp->next;
                            }
                        }
                        else if (FD_ISSET(i, &tmp_write_fds)) {
                            //send data
                            client_sock=find_socket_node(httpd->head_node,i);
//                            r = handle_response_with_handler(client_sock, httpd);
                            tmp = client_sock->w;
                            while(tmp) {
                                if(tmp->events&IO_WRITE) {
                                    func = tmp->watcher_cb;
                                    r = func(client_sock, httpd);
                                }
                                if (r == IO_DONE_W || r == IO_ERROR) {
                                    shutdown(i, SHUT_WR);
                                    FD_CLR(i, &write_fds);
                                    FD_SET(i, &read_fds);
                                }
                                else if (r == IO_EAGAIN_W) {
                                    printf("WARNNING: write EAGAIN code.\n");
                                }
                                else if (r == IO_EAGAIN_R) {
                                    FD_SET(i, &read_fds);
                                    FD_CLR(i, &write_fds); //重新加入select中
                                    printf("WARNNING: read EAGAIN code.\n");
                                }
                                tmp = tmp->next;
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
                        printf("> [socket-%d] accept : %s\n", client_fd, ipaddr);
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