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

#define INT_32 int
#define BACKLOG 20
#define MAX_CLIENTS 1024
#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define MAX_PATH_LENGTH 256
#define SERVER_STRING "Server: jmpews-httpd/0.1.0\r\n"

struct clinfo *clients[MAX_CLIENTS];
char rootpath[50];

#define is_space(x) isspace((int)(x))

INT_32 init_socket(INT_32 *listen_fd, struct sockaddr_in *server_addr);
INT_32 read_data(INT_32 fd,char *buffer);
void send_data(INT_32 fd,const char *buffer);
int accept_request(int client_fd);
void send_file(int client_fd, FILE *fd);
int get_line(int sock, char *buf, int size);
void send_headers(int client_fd);
void send_not_found(int client);
void serve_file(int client_fd,const char *filename);
INT_32 startup(int *port);
void send_response(int client_fd);
void epoll_close(int epoll_fd,int fd,struct epoll_event *ev);
void start_epoll_loop(int httpd);

#endif /* sockets_h */


typedef struct socketnode{
    int client_fd;
    char *filepath;
    int filed;
    struct client *next;
}SocketNode;

SocketNode *SocketHeader;

SocketNode * new_socket_node(){
    SocketNode *tmp=(SocketNode *)malloc(sizeof(SocketNode));
    tmp->client_fd=-1;
    tmp->filed=0;
    tmp->next=NULL;
    tmp->filepath=NULL;
    // memset(tmp->filepath,0,sizeof(char)*MAX_PATH_LENGTH);
}

SocketNode * find_socket_node(int client_fd)
{
    SocketNode *tmp=SocketHeader;
    while(1)
        if(tmp!=NULL)
            if(tmp_read_fds->client_fd==client_fd)
                return tmp;
            else
                tmp=tmp->next;
        else
            return NULL;

}

void add_socket_node(SocketNode *client)
{
    SocketNode *tmp=SocketHeader;
    client->next=tmp;
    SocketHeader=client;
}

void free_socket_node(int client_fd)
{
    SocketNode *tmp;
    SocketNode *k;
    while(1)
        if(tmp!=NULL)
            if(tmp->next->client_fd==client_fd)
                break
    if(tmp==NULL)
    {
        printf("! free_socket_node ERROR\n");
        close(client_fd);
        return;
    }
    k=tmp->next;
    tmp->next=k->next;
    free(k);
    close(client_fd);
}
INT_32 set_nonblocking(INT_32 sockfd)
{
    INT_32 opts;
    opts = fcntl(sockfd, F_GETFL);
    if(opts < 0) {
        printf("! fcntl: F_GETFL");
        return -1;
    }
    
    opts = opts | O_NONBLOCK;
    if( fcntl(sockfd, F_SETFL, opts) < 0 ) {
        printf("! fcntl: F_SETFL");
        return -1;
    }
    
    printf("> set socket %d to nonblocking.\n", sockfd);
    
    return 0;
}



INT_32 startup(int *port)
{
    int httpd=0;
    struct sockaddr_in server_addr;
    if((httpd=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        printf("! httpd start error.");
        exit(1);
    }
    
    INT_32 opt = SO_REUSEADDR;
    if(setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        printf("! httpd reuseaddr error.");
        exit(1);
    }
    
    //set no-blocking
    set_nonblocking(httpd);

    //new socket_info
    SocketNode *tmp=new_socket_node();
    tmp->client_fd=httpd;
    add_client_node(tmp);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(httpd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("! httpd bind error.\n");
        return -1;
    }
    
    if(listen(httpd, BACKLOG) == -1)
    {
        printf("! httpd listen error.\n");
        return -1;
    }
    getcwd(rootpath, sizeof(rootpath));
    return httpd;
}

int get_line(int sock, char *buf, int size){
    int i=0;
    char c='\0';
    int r;
    /* 转换/r/n 到 /n */
    while ((i<size-1)&&(c!='\n')) {
        r=recv(sock, &c, 1, 0);
        if(r>0)
        {
            if(c=='\r')
            {
                /* 从缓冲区copy数据，并不删除数据，如果符合再次读取数据 */
                r=recv(sock,&c,1,MSG_PEEK);
                if (r>0&&c=='\n')
                    recv(sock, &c, 1, 0);
                else
                    c='\n';
            }
            buf[i]=c;
            i++;
        }
        else
            c='\n';
    }
    buf[i]='\0';
    return i;
}

int accept_request(int client_fd)
{
    char buf[1024];
    char method[255];
    char url[255];
    int r;
    struct stat st;
    char wwwpath[256];
    int i=0,j=0;
    char *query_string = NULL;
    struct clinfo *cli;



    r=get_line(client_fd, buf, sizeof(buf));
    if(r==0)
    {
        //读取len 0数据,close
        return 0;
    }
    //read request method
    while (!(is_space(buf[j]))&&i<sizeof(buf))
    {
        method[i]=buf[j];
        i++;j++;
    }
    method[i]='\0';
    if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))
    {
        printf("! request method not support.\n");
        return -1;
    }
    while (is_space(buf[j])&&(j<sizeof(buf)))
        j++;
    
    i=0;
    while (!is_space(buf[j])&&(j<sizeof(buf)))
    {
        url[i]=buf[j];
        i++;j++;
    }
    url[i]='\0';
    //request get method
    if (strcasecmp(method,"GET")==0)
    {
        query_string=url;
        while ((*query_string!='?')&&(*query_string!='\0'))
            query_string++;
        if (*query_string=='?') {
            printf("read get args.");
            *query_string='\0';
            query_string++;
        }
    }
    
    printf("\nSOCKET-%d USER HEADER:\n",client_fd);
    while ((r>0)&&strcmp("\n", buf))
    {
        r=get_line(client_fd, buf, sizeof(buf));
        j+=r;
        printf("%s",buf);
    }

    sprintf(wwwpath, "%s/htdocs%s",rootpath,url);
    if (wwwpath[strlen(wwwpath)-1]=='/')
        strcat(wwwpath, "index.html");
    else{
        if((st.st_mode&S_IFMT)==S_IFDIR)
            strcat(wwwpath, "/index.html");
    }
    if((stat(wwwpath,&st)!=-1)&&((st.st_mode&S_IFMT)!=S_IFREG))
    {
        SocketNode * tmp= find_client_node(client_fd);
        tmp->filepath=(char *)malloc(strlen(wwwpath)*sizeof(char));
        strcpy(tmp->filepath,wwwpath);
    }
    wwwpath[0]='\0';
    return j;
}

void send_response(int client_fd)
{
    if(clients[client_fd]->filepath!=NULL)
        serve_file(client_fd, clients[client_fd]->filepath);
    else
        send_not_found(client_fd);
}


void send_headers(int client_fd)
{
    
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    
    strcat(buf, "HTTP/1.0 200 OK\r\n");
    strcat(buf, SERVER_STRING);
    strcat(buf, "Content-Type: text/html\r\n");
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

void send_file(int client_fd, FILE *fd)
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), fd);
    while (!feof(fd))
    {
        send(client_fd, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), fd);
    }
}

void serve_file(int client_fd,const char *filename)
{
    FILE *fd=NULL;
    printf("> Read File%s\n",filename);
    fd=fopen(filename, "r");
    if (fd==NULL)
        send_not_found(client_fd);
    else {
        send_headers(client_fd);
        send_file(client_fd, fd);
    }
    fclose(fd);
}


void send_not_found(int client)
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    strcat(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    strcat(buf, SERVER_STRING);
    strcat(buf, "Content-Type: text/html\r\n");
    strcat(buf, "\r\n");
    strcat(buf, "<!DOCTYPE html>\r\n<html lang=\"en\">");
    strcat(buf, "<head>\r\n<meta charset=\"UTF-8\">\r\n<title>Not Found</title></head>\r\n");
    strcat(buf, "<body>\r\n<h2 style=\"text-align: center;\"> not found world.</h2>");
    strcat(buf, "<h5 style=\"text-align: right;\">by jmpews.</h5>\r\n");
    strcat(buf, "</body></html>");
    buf[strlen(buf)]='\0';
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

void start_epoll_loop(int httpd)
{
    int epoll_fd,nfds;
    int client_fd;
    int s;
    int i;
    int header_len=0;
    struct epoll_event ev;
    struct epoll_event *events;
    struct sockaddr_in client_addr;
    socklen_t socket_len= sizeof(struct sockaddr_in);

    epoll_fd=epoll_create(MAX_CLIENTS);
    ev.data.fd = httpd;
    ev.events = EPOLLIN|EPOLLET;
    s=epoll_ctl(epoll_fd, EPOLL_CTL_ADD, httpd, &ev);
    if (s == -1)
    {
        printf("! epoll_ctl error.\n");
        abort();
    }
    events=(struct epoll_event*)malloc(MAX_CLIENTS*sizeof(struct epoll_event));
    if(events==NULL)
    {
        printf("! malloc error.\n");
    }
    memset(events,0,MAX_CLIENTS*sizeof(struct epoll_event));
    for (; ;) {
        nfds=epoll_wait(epoll_fd, events, MAX_CLIENTS, 3000);
        for (i = 0; i < nfds; i++)
        {
            if((events[i].events&EPOLLERR) || (events[i].events & EPOLLHUP))
            {
                printf("! epoll error.\n");
                //abort();
                continue;
            }
            else if(events[i].events&EPOLLIN)
            {
                if(events[i].data.fd==httpd)
                {
                    client_fd= accept(httpd, (struct sockaddr *)&client_addr,&socket_len);
                    set_nonblocking(client_fd);
                    ev.data.fd = client_fd;
                    ev.events = EPOLLIN|EPOLLET;
                    s=epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                    if (s == -1)
                        epoll_close(epoll_fd,events[i].data.fd,&ev);
                }
                else
                {
                    header_len=accept_request(events[i].data.fd);
                    if(header_len>0)
                    {
                        ev.data.fd = events[i].data.fd;
                        ev.events = EPOLLOUT|EPOLLET;
                        s=epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                        if(s==-1)
                            epoll_close(epoll_fd,events[i].data.fd,&ev);
                    }
                    else
                    {
                        epoll_close(epoll_fd,events[i].data.fd,&ev);
                    }
                }
            }
            else if(events[i].events&EPOLLOUT)
            {
                printf("> close socket %d\n",events[i].data.fd);
                send_response(events[i].data.fd);
                epoll_close(epoll_fd,events[i].data.fd,&ev);

            }
        }
    }
}

void epoll_close(int epoll_fd,int fd,struct epoll_event *ev)
{

    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, ev)==-1)
        printf("! close epoll_ctl error.\n");
    free_socket_node(fd);
}

void start_select_loop(int httpd)
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

    /* wait for new connect */
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
