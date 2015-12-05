//
//  sockets.h
//  netcore
//
//  Created by jmpews on 15/11/6.
//  Copyright © 2015年 jmpews. All rights reserved.
//
//1. 每一个while除了需要判断需要的条件，还需要限制循环次数(即for的第三个参数)。
//2. string的结尾字符\0
//3. recv的MSG_PEEK参数，仅仅copy读取的buff，并不删除。
//4. malloc 要考虑所有可能的异常情况然后free
//5. segmentation fault (core dumped),内存问题,有没有初始化等等!
//6. strcp的坑
// malloc() 和""不一样，对于'\0',""默认长度+1
/*  char * file1=(char *)malloc(2*sizeof(char));
    file1[0]='a';
    file1[1]='b';
    file1[2]='c';
    char * file2="ab";
    printf("ABequal:%d\n",strcmp(file1,file2));
    */

//7. Content-Type: 有坑,要注意设置.
//8. 对于错误处理要果断,如果该错误是自己assert不会出错,那就直接打印error code然后exit,不要强行继续处理.
//9. 发送缓冲区的处理,8k左右字节,read了多少数据,就send多少数据,如果send返回缓冲区满了异常,那就重新将事件丢入,循环.
//10.规范化状态码
//11.header状态码处理
//12.如何读一个长度很大并且未知的数据,先分配一个缓存char buf[1024],每次读到buf,并且记录每次读取的数量,然后realloc重新分配空间,直至终点
//13.读取body一行没有\n最后的处理
#ifndef sockets_h
#define sockets_h

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// 状态码标准化
#define IO_DONE 0
#define IO_CONTINUE 1
#define IO_YET -1
#define IO_ERROR -2


#define ERROR_CODE -1
#define SUCCESS_CODE 0

#define GET 1
#define POST 2

// header处理状态
#define REQ_NO -1
#define REQ_INIT 1
#define REQ_START 2
#define REQ_HEAD_OVER 3
#define REQ_BODY 4
#define REQ_HOST 5


#define INT_32 int
#define BACKLOG 20
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024
#define SERVER_STRING "Server: jmp2httpd 0.1.0\r\n"

#define is_space(x) isspace((INT_32)(x))
#define TIP if(1){}else
#define LOG(t) {printf("log[%d]..........\n",t);fflush(stdout);}
#define FREEBUF(buf) if(buf){free(buf);buf=NULL;}
#define PRINT_LINE_TITLE(str) printf("\n----------------%s----------------\n", str);

#define LOGERRNO printf("EGAGAIN=[%d],CURERRNO=[%d]",EAGAIN,errno);fflush(stdout);

char rootpath[50];


INT_32 handle_request(INT_32 client_fd);

void send_headers(INT_32 client_fd);

void send_not_found(INT_32 client);

INT_32 startup(INT_32 *port);

INT_32 send_response(INT_32 client_fd);

void epoll_close(INT_32 epoll_fd, INT_32 fd, struct epoll_event *ev);

void start_epoll_loop(INT_32 httpd);

typedef struct readbuf {
    char key[32];
    char status;
    char *value;
    long content_length;
    long current_length;
} ReadBuf;

typedef struct sendbuf {
    long content_length;
    long current_length;
} SendBuf;
typedef struct header {
    INT_32 method;
    char *request_path;
} ReqHeader;

//connect链表
typedef struct snode {

    INT_32 client_fd;
    ReqHeader Header;
    //read状态
    char RS;
    ReadBuf RBuf;

    //send状态
    char SS;
    SendBuf SBuf;

    struct snode *next;
} SocketNode;

INT_32 get_line(INT_32 sock, SocketNode *tmp);

INT_32 send_file(INT_32 client_fd, SocketNode *tmp);

SocketNode *find_socket_node(INT_32 client_fd);

void add_socket_node(SocketNode *client);

INT_32 read_line(INT_32 client_fd, SocketNode *tmp);

void check_socket_list();

void free_socket_node(INT_32 client_fd);

#endif /* sockets_h */
SocketNode *SocketHeader;

SocketNode *new_socket_node() {
    SocketNode *tmp = (SocketNode *) malloc(sizeof(SocketNode));
    memset(tmp, 0, sizeof(SocketNode));
    return tmp;
    // memset(tmp->filepath,0,sizeof(char)*MAX_PATH_LENGTH);
}

SocketNode *find_socket_node(INT_32 client_fd) {
    SocketNode *tmp = SocketHeader;
    if (SocketHeader == NULL) {
        printf("! socketheader null\n");
        exit(1);
    }
    while (tmp != NULL) {
        if (tmp->client_fd == client_fd)
            return tmp;
        else
            tmp = tmp->next;
    }
    return NULL;

}

void add_socket_node(SocketNode *client) {
    // 添加节点到HeaderNode与其他Node之间
    client->next = SocketHeader->next;
    SocketHeader->next = client;
}

void check_socket_list() {
    SocketNode *tmp = SocketHeader;
    TIP printf("> check_socket_list\n");
    while (tmp) {
        TIP printf("SOCKET[%d] live\n", tmp->client_fd);
        tmp = tmp->next;
    }
}

void free_socket_node(INT_32 client_fd) {
    TIP printf("> SOCKET[%d] free.\n", client_fd);
    SocketNode *tmp = SocketHeader;
    SocketNode *k;
    //空链表
    if (tmp == NULL) {
        printf("! free_socket_node ERROR\n");
        exit(1);
    }
    while ((tmp->next != NULL)) {
        if (tmp->next->client_fd == client_fd)
            break;
        tmp = tmp->next;
    }
    k = tmp->next;
    //没找到node
    if (k == NULL) {
        printf("! free_socket_nod not found client_fd\n");
        close(client_fd);
        exit(1);
    }
    tmp->next = k->next;
    FREEBUF(k->RBuf.value);
    FREEBUF(k->Header.request_path);
    FREEBUF(k);
    close(client_fd);
}

INT_32 set_nonblocking(INT_32 sockfd) {
    INT_32 opts;
    opts = fcntl(sockfd, F_GETFL);
    if (opts < 0) {
        printf("! fcntl: F_GETFL");
        return -1;
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0) {
        printf("! fcntl: F_SETFL");
        return -1;
    }

    TIP printf("> Socket[%d] non-blocking.\n", sockfd);

    return 0;
}

INT_32 startup(INT_32 *port) {
    INT_32 httpd = 0;
    struct sockaddr_in server_addr;
    if ((httpd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("! httpd start error.");
        exit(1);
    }

    INT_32 opt = SO_REUSEADDR;
    if (setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("! Reuse error.");
        exit(1);
    }

    //set no-blocking
    set_nonblocking(httpd);

    //new socket_info
    SocketNode *tmp = new_socket_node();
    tmp->client_fd = httpd;
    //add_socket_node(tmp);
    SocketHeader = tmp;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(httpd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("! Bind error:\n");
        exit(1);
    }

    if (listen(httpd, BACKLOG) == -1) {
        printf("! Listen error.\n");
        exit(1);
    }
    memset(rootpath, 0, sizeof(rootpath));
    getcwd(rootpath, sizeof(rootpath));
    return httpd;
}

INT_32 get_line(INT_32 sock, SocketNode *tmp) {
    INT_32 i, j, n;
    char c = '\0';
    INT_32 r;
    INT_32 buf_size = 1024;
    char buf[buf_size];
    char *value;
    if (tmp->RS == IO_YET) {
        // 恢复上一次状态
        tmp->RS == IO_DONE;
        value = tmp->RBuf.value;
        n = tmp->RBuf.current_length;
    }
    else {
        // free上一次
        FREEBUF(tmp->RBuf.value);
        n = 0;
    }
    while (1) {
        i = 0;
        /* 转换/r/n 到 /n */
        while ((i < buf_size - 1) && (c != '\n')) {
            r = recv(sock, &c, 1, 0);
            // printf("%c", c);
            // fflush(stdout);
            if (r > 0) {
                if (c == '\r') {
                    /* 从缓冲区copy数据，并不删除数据，如果符合再次读取数据 */
                    r = recv(sock, &c, 1, MSG_PEEK);
                    if (r > 0 && c == '\n')
                        recv(sock, &c, 1, 0);
                    else
                        c = '\n';

                    buf[i++] = c;
                    break;
                }
                buf[i] = c;
                i++;
            }
            else {
                if (errno == EAGAIN)
                    break;
            }
        }
        // buf[i]='\0';
        //如果读取0字节,并且EAGAIN,表明读取完毕,提前判断避免malloc
        if (i == 0 && r < 0 && errno == EAGAIN) {
            //多余
            FREEBUF(tmp->RBuf.value);
            return IO_DONE;
        }
        // 分配一块内存,如果这一行过大,要支持realloc
        if (n)
            value = (char *) realloc(value, (n + i + 1) * sizeof(char));
        else
            value = (char *) malloc((i + 1) * sizeof(char));
        //复制读取数据
        memcpy(value + n, buf, i * sizeof(char));
        n += i;
        value[n] = '\0';
        tmp->RBuf.value = value;
        //EAGAIN
        if (r < 0) {
            if (errno == EAGAIN) {
                // 这里特殊情况处理下，当读到HEAD_OVER
                if (tmp->RBuf.status==REQ_HEAD_OVER)
                    return n;
                tmp->RS = IO_YET;
                return IO_YET;
            }
            else {
                perror("! get_line error");
                exit(1);
            }
        }
        else if (r >= 0 && c == '\n') {
            return n;
        }
        else{
            //表明这一行数据太多,继续读取,realloc.
        }
    }
}

INT_32 handle_header(INT_32 client_fd, SocketNode *tmp) {
    INT_32 i = 0, r = 0;
    INT_32 key_size = 32;

    char *buf;
    r = get_line(client_fd, tmp);
    buf = tmp->RBuf.value;
    //故意=0
    while (r > 0) {
        //打印请求头
        printf("%s", buf);
        i = 0;
        while (i < r && i < key_size)
            if (buf[i] == ':')
                break;
            else
                i++;
        i += 1;
        //tmp->RBuf.value = (char *) malloc(sizeof(char) * (r - i));
        //memcpy(tmp->RBuf.value, buf + i, r - i);
        if (!strcmp(tmp->RBuf.value, "\n")) {
            tmp->RBuf.status = REQ_HEAD_OVER;
            PRINT_LINE_TITLE("header-end");
            // printf("-------header end------\n");
            // r = get_line(client_fd, tmp);
            // buf = tmp->RBuf.value;
            // printf("%s:%d", buf,r);
        }
        else if (tmp->RBuf.status == REQ_HEAD_OVER) {
            //body的判断方式
            tmp->RBuf.status = REQ_BODY;
            PRINT_LINE_TITLE("body-end");
            // printf("\n-------body end------\n");
            return IO_DONE;
        }
        //释放,多余
        //free(tmp->RBuf.value);
        r = get_line(client_fd, tmp);
        buf = tmp->RBuf.value;
    }
    if (r == IO_DONE) {
        return IO_DONE;
    }
    else
        return IO_YET;
}

INT_32 handle_request(INT_32 client_fd) {
    SocketNode *tmp;
    INT_32 r;
    INT_32 i, j;
    char method[32];
    char url[256];
    char request_path[256];
    char *buf;
    memset(method, 0, 256 * sizeof(char));
    memset(url, 0, 256 * sizeof(char));
    memset(request_path, 0, 256 * sizeof(char));

    tmp = find_socket_node(client_fd);
    if (tmp->RS != IO_YET) {
        //start read
        r = get_line(client_fd, tmp);
        buf = tmp->RBuf.value;
        if (r == IO_YET) {
            return IO_YET;
        }
        else {
            tmp->RBuf.status = -REQ_START;
            PRINT_LINE_TITLE("header-start");
            // printf("-------header start------\n");
            printf("%s", buf);

        }
        //设置请求方法
        i = j = 0;
        while (!(is_space(buf[j])) && i < BUFFER_SIZE) {
            method[i] = buf[j];
            i++;
            j++;
        }
        method[i] = '\0';
        if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
            printf("! request method not support.\n");
            exit(1);
        }
        if (!strcasecmp(method, "GET"))
            tmp->Header.method = GET;
        else if (!strcasecmp(method, "POST"))
            tmp->Header.method = POST;

        while (is_space(buf[j]) && (j < BUFFER_SIZE))
            j++;

        //设置请求路径
        i = 0;
        while (!is_space(buf[j]) && (j < BUFFER_SIZE)) {
            url[i] = buf[j];
            i++;
            j++;
        }
        sprintf(request_path, "%s/htdocs%s", rootpath, url);
        tmp->Header.request_path = (char *) malloc((strlen(request_path) + 11) * sizeof(char));
        strcpy(tmp->Header.request_path, request_path);
        tmp->Header.request_path[strlen(request_path)] = '\0';
    }

    // 处理后续header
    r = handle_header(client_fd, tmp);
    if (r == IO_YET)
        return IO_YET;
    else {
        return IO_DONE;
    }
}

INT_32 send_response(INT_32 client_fd) {
    struct stat st;
    SocketNode *tmp;
    char *path;
    INT_32 r;
    tmp = find_socket_node(client_fd);

    if (tmp == NULL) {
        printf("! find_socket_node error.");
        exit(1);
    }
    if (tmp->SS == IO_YET) {
        // 没有发送完毕,无需判断,直接发送文件
        r = send_file(client_fd, tmp);
        return r;
    }
    else if (tmp->Header.request_path != NULL) {
        path = tmp->Header.request_path;
        if (path[strlen(path) - 1] == '/')
            strcat(path, "index.html");
        path[strlen(path)] = '\0';
        if ((stat(path, &st) != -1) && ((st.st_mode & S_IFMT) == S_IFREG)) {
            r = send_file(client_fd, tmp);
            return r;
        }
    }
    send_not_found(client_fd);
    return IO_DONE;
}


void send_headers(INT_32 client_fd) {
    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);
    strcat(buf, "HTTP/1.0 200 OK\r\n");
    strcat(buf, SERVER_STRING);
    //strcat(buf, "Content-Type: text/html\r\n");
    strcat(buf, "\r\n");
    send(client_fd, buf, strlen(buf), 0);


    /*
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client_fd, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client_fd, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client_fd, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client_fd, buf, strlen(buf), 0);
    */
}

INT_32 send_file(INT_32 client_fd, SocketNode *tmp) {
    //if((st.st_mode&S_IFMT)==S_IFDIR)
    FILE *fd;
    long file_length = 0;
    char buf[BUFFER_SIZE];
    size_t r = 0;
    long t;

    fd = fopen(tmp->Header.request_path, "r");
    if (fd == NULL) {
        perror("! send_file/fopen error\n");
        exit(1);
    }
    fseek(fd, 0, SEEK_END);
    file_length = ftell(fd);
    rewind(fd);

    //设置文件当前指针,为上次没有读完的
    if (tmp->SS == IO_YET)
        fseek(fd, tmp->SBuf.current_length, SEEK_SET);
    else
        send_headers(client_fd);
    memset(buf, 0, BUFFER_SIZE);
    r = fread(buf, sizeof(char), BUFFER_SIZE, fd);
    while (1) {
        t = send(client_fd, buf, r, 0);
        if (t < 0) {
            if (errno == EAGAIN) {
                TIP printf("! EAGAIN");
                return IO_YET;
            }
            else {
                printf("! Send Error:");
                exit(1);
            }
        }
        tmp->SBuf.current_length += t;
        memset(buf, 0, BUFFER_SIZE);
        r = fread(buf, sizeof(char), BUFFER_SIZE, fd);
        if (r == 0)
            return IO_DONE;
    }
}


void send_not_found(INT_32 client) {
    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);
    strcat(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    strcat(buf, SERVER_STRING);
    strcat(buf, "Content-Type: text/html\r\n");
    strcat(buf, "\r\n");
    strcat(buf, "<!DOCTYPE html>\r\n<html lang=\"en\">");
    strcat(buf, "<head>\r\n<meta charset=\"UTF-8\">\r\n<title>Not Found</title></head>\r\n");
    strcat(buf, "<body>\r\n<h2 style=\"text-align: center;\"> not found world.</h2>");
    strcat(buf, "<h5 style=\"text-align: right;\">by jmpews.</h5>\r\n");
    strcat(buf, "</body></html>");
    buf[strlen(buf)] = '\0';
    send(client, buf, strlen(buf), 0);

    /*
    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
    */
}

void start_epoll_loop(INT_32 httpd) {
    INT_32 epoll_fd, nfds;
    INT_32 client_fd;
    INT_32 s;
    INT_32 i;
    INT_32 r;
    struct epoll_event ev;
    struct epoll_event *events;
    struct sockaddr_in client_addr;
    char ipaddr[32];
    socklen_t socket_len = sizeof(struct sockaddr_in);

    epoll_fd = epoll_create(MAX_CLIENTS);
    ev.data.fd = httpd;
    ev.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, httpd, &ev);
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

                if (events[i].data.fd == httpd) {
                    //ET until get the errno=EAGAIN
                    while ((client_fd = accept(httpd, (struct sockaddr *) &client_addr, &socket_len)) > 0) {
                        inet_ntop(AF_INET, &(client_addr.sin_addr), ipaddr, 32 * sizeof(char));
                        printf("> SOCKET[%d] Accept : %s\n", client_fd, ipaddr);
                        set_nonblocking(client_fd);
                        ev.data.fd = client_fd;
                        ev.events = EPOLLIN | EPOLLET;
                        s = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                        if (s == -1)
                            epoll_close(epoll_fd, events[i].data.fd, &ev);
                        SocketNode *tmp = new_socket_node();
                        tmp->client_fd = client_fd;
                        add_socket_node(tmp);
                    }
                    if (errno == EAGAIN) TIP printf("! accept EAGAIN\n");
                        //printf("! EAGAIN try again\n");
                    else
                        perror("! Accept Error:");
                    continue;
                }
                else {
                    r = handle_request(events[i].data.fd);
                    if (r == IO_DONE) {
                        ev.data.fd = events[i].data.fd;
                        ev.events = EPOLLOUT | EPOLLET;
                        s = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                        if (s == -1)
                            epoll_close(epoll_fd, events[i].data.fd, &ev);
                    }
                    else if (r == IO_YET) {
                    }
                    else {
                        epoll_close(epoll_fd, events[i].data.fd, &ev);
                    }
                }
            }
            else if (events[i].events & EPOLLOUT) {
                r = send_response(events[i].data.fd);
                if (IO_DONE == r)
                    epoll_close(epoll_fd, events[i].data.fd, &ev);

            }
            else {
                printf("uncatched.");
                epoll_close(epoll_fd, events[i].data.fd, &ev);
            }
        }
    }
}

void epoll_close(INT_32 epoll_fd, INT_32 fd, struct epoll_event *ev) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, ev) == -1)
        printf("! close epoll_ctl error.\n");
    free_socket_node(fd);
    printf("> Socket[%d] close.\n", fd);
}

/*
void start_select_loop(INT_32 httpd)
{
    INT_32 connect_fd;
    INT_32 retcode;
    INT_32 result;
    INT_32 maxfd;
    INT_32 i;
    INT_32 checks[MAX_CLIENTS];
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t addr_len=sizeof(struct sockaddr_in);
    memset(checks, 0, sizeof(checks));
    fd_set read_fds;
    fd_set write_fds;
    fd_set exception_fds;
    fd_set tmp_read_fds;
    fd_set tmp_write_fds;
    fd_set tmp_exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&exception_fds);

    // wait for new connect
    FD_SET(httpd,&read_fds);
    checks[httpd]=1;

    struct timeval tv;
    tv.tv_sec=5;
    tv.tv_usec=0;
    while (1) {
        tmp_read_fds=read_fds;
        tmp_write_fds=write_fds;
        tmp_exception_fds=exception_fds;
        maxfd=0;

        for (i=0; i<MAX_CLIENTS; i++)
        {
            if (checks[i])
            if (maxfd<i)
                maxfd=i;
        }

        retcode=select(maxfd+1, &tmp_read_fds, &tmp_write_fds, &tmp_exception_fds, &tv);
        if (retcode<0)
            printf("! select() error.");
        else if(retcode==0)
        {
            continue;
        } else {
            // new connect
            if (FD_ISSET(httpd,&tmp_read_fds)) {
                connect_fd=accept(httpd, (struct sockaddr*)&client_addr, &addr_len);
                struct clinfo *cli=(struct clinfo*)malloc(sizeof(struct clinfo));
                cli->client_fd=connect_fd;
                clients[connect_fd]=cli;

                if (connect_fd<0)
                    printf("! client connect error.");
                else
                {
                    printf("> connect %d comming.\n",connect_fd);
                    set_nonblocking(connect_fd);
                    FD_SET(connect_fd,&read_fds);
                    checks[connect_fd]=1;
                    maxfd=(maxfd < connect_fd)?connect_fd:maxfd;
                }
            } else{
                for(i=httpd+1;i<=maxfd;i++){
                    if(checks[i])
                    {
                        if(FD_ISSET(i,&tmp_read_fds))
                        {
                            //read data or close connect
                            result=accept_request(i);
                            if(result==1)
                            {
                                FD_CLR(i,&read_fds);
                                FD_SET(i,&write_fds);
                            } else if (result == 0||result==-1) {
                                close(i);
                                FD_CLR(i, &read_fds);
                                checks[i]=0;
                                free(clients[i]);
                            }
                        }
                        else if(FD_ISSET(i,&tmp_write_fds))
                        {
                            //send data
                            send_response(i);
                            close(i);
                            FD_CLR(i,&write_fds);
                            checks[i]=0;
                            free(clients[i]);
                        }
                    }
                }
            }
        }
    }
}
*/
