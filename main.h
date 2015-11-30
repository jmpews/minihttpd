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
#define INT_32 int
#define BACKLOG 20
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024
#define SERVER_STRING "Server: jmpews-httpd/0.1.0\r\n"

struct clinfo *clients[MAX_CLIENTS];
char rootpath[50];

#define is_space(x) isspace((int)(x))
#define TIP if(1){}else

INT_32 read_data(INT_32 fd,char *buffer);
void send_data(INT_32 fd,const char *buffer);
int accept_request(int client_fd);

int get_line(int sock, char *buf, int size);
void send_headers(int client_fd);
void send_not_found(int client);
INT_32 startup(int *port);
long send_response(int client_fd);
void epoll_close(int epoll_fd,int fd,struct epoll_event *ev);
void start_epoll_loop(int httpd);

//connect链表
typedef struct snode
{
    int client_fd;
    char *filepath;
    long slen;
    struct snode *next;
} SocketNode;
long send_file(int client_fd, SocketNode *tmp);
SocketNode *find_socket_node(int client_fd);
void add_socket_node(SocketNode *client);
void check_socket_list();

void free_socket_node(int client_fd);

#endif /* sockets_h */
SocketNode *SocketHeader;

SocketNode *new_socket_node()
{
    SocketNode *tmp=(SocketNode *)malloc(sizeof(SocketNode));
    tmp->client_fd=-1;
    tmp->slen=-1;
    tmp->next=NULL;
    tmp->filepath=NULL;
    return tmp;
    // memset(tmp->filepath,0,sizeof(char)*MAX_PATH_LENGTH);
}

SocketNode *find_socket_node(int client_fd)
{
    SocketNode *tmp=SocketHeader;
    if(SocketHeader==NULL)
    {
        printf("! socketheader null\n");
        exit(1);
    }
    while(tmp!=NULL)
    {
        if(tmp->client_fd==client_fd)
            return tmp;
        else
            tmp=tmp->next;
    }
    return NULL;

}

void add_socket_node(SocketNode *client)
{
    // add the new node at Header behind and before others
    // SocketNode *tmp=SocketHeader;
    client->next=SocketHeader->next;
    SocketHeader->next=client;
}

void check_socket_list()
{
    SocketNode *tmp=SocketHeader;
    TIP printf("> check_socket_list\n");
    while(tmp)
    {
        TIP printf("SOCKET[%d] live\n",tmp->client_fd);
        tmp=tmp->next;
    }
}

void free_socket_node(int client_fd)
{
    TIP printf("> SOCKET[%d] free.\n",client_fd);
    SocketNode *tmp=SocketHeader;
    SocketNode *k;
    //空链表
    if(tmp==NULL)
    {
        printf("! free_socket_node ERROR\n");
        exit(1);
    }
    //链表头是要找的node
    /*
    if(tmp->client_fd==client_fd)
    {
        SocketHeader=tmp->next;
        free(tmp);
        close(client_fd);
        return;
    }
    */
    while((tmp->next!=NULL))
    {
        if(tmp->next->client_fd==client_fd)
            break;
        tmp=tmp->next;
    }
    k=tmp->next;
    //没找到node
    if(k==NULL)
    {
        printf("! free_socket_nod not found client_fd\n");
        close(client_fd);
        return;
    }
    tmp->next=k->next;
    free(k);
    close(client_fd);
}
INT_32 set_nonblocking(INT_32 sockfd)
{
    INT_32 opts;
    opts = fcntl(sockfd, F_GETFL);
    if(opts < 0)
    {
        printf("! fcntl: F_GETFL");
        return -1;
    }

    opts = opts | O_NONBLOCK;
    if( fcntl(sockfd, F_SETFL, opts) < 0 )
    {
        printf("! fcntl: F_SETFL");
        return -1;
    }

    TIP printf("> Socket[%d] non-blocking.\n", sockfd);

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
    //add_socket_node(tmp);
    SocketHeader=tmp;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(httpd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1)
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

int get_line(int sock, char *buf, int size)
{
    int i=0;
    char c='\0';
    int r;
    /* 转换/r/n 到 /n */
    while ((i<size-1)&&(c!='\n'))
    {
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
    char buf[BUFFER_SIZE];
    char method[BUFFER_SIZE];
    char url[255];
    int r;
    char wwwpath[256];
    int i=0,j=0;
    char *query_string = NULL;
    SocketNode *tmp=NULL;

    r=get_line(client_fd, buf, BUFFER_SIZE);
    if(r==0)
    {
        //读取len 0数据,close
        return 0;
    }
    printf(".................................\nSocket[%d] Header:\n",client_fd);
    printf("%s",buf);
    //read request method
    while (!(is_space(buf[j]))&&i<BUFFER_SIZE)
    {
        method[i]=buf[j];
        i++;
        j++;
    }
    method[i]='\0';
    if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))
    {
        printf("! request method not support.\n");
        return -1;
    }
    while (is_space(buf[j])&&(j<BUFFER_SIZE))
        j++;

    i=0;
    while (!is_space(buf[j])&&(j<BUFFER_SIZE))
    {
        url[i]=buf[j];
        i++;
        j++;
    }
    url[i]='\0';
    //request get method
    if (strcasecmp(method,"GET")==0)
    {
        query_string=url;
        while ((*query_string!='?')&&(*query_string!='\0'))
            query_string++;
        if (*query_string=='?')
        {
            printf("read get args.");
            *query_string='\0';
            query_string++;
        }
    }


    while ((r>0)&&strcmp("\n", buf))
    {
        r=get_line(client_fd, buf, BUFFER_SIZE);
        j+=r;
        printf("%s",buf);
    }

    sprintf(wwwpath, "%s/htdocs%s",rootpath,url);
    tmp= find_socket_node(client_fd);
    if(tmp==NULL)
    {
        printf("! find_socket_node null\n");
        exit(1);
    }
    tmp->filepath=(char *)malloc((strlen(wwwpath)+11)*sizeof(char));
    strcpy(tmp->filepath,wwwpath);
    tmp->filepath[strlen(wwwpath)]='\0';
    return j;
}

long send_response(int client_fd)
{
    struct stat st;
    SocketNode *tmp;
    char *path;
    long r;
    tmp=find_socket_node(client_fd);

    if(tmp==NULL)
    {
        printf("! find_socket_node error.");
        exit(1);
    }
    if(tmp->slen>0)
    {
        r=send_file(client_fd,tmp);
        return r;
    }
    else if(tmp->filepath!=NULL)
    {
        path=tmp->filepath;
        if(path[strlen(path)-1]=='/')
            strcat(path,"index.html");
        path[strlen(path)]='\0';
        if((stat(path,&st)!=-1)&&((st.st_mode&S_IFMT)==S_IFREG))
        {
            r=send_file(client_fd,tmp);
            return r;
        }
    }
    send_not_found(client_fd);
    return 0;
}


void send_headers(int client_fd)
{
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

long send_file(int client_fd, SocketNode *tmp)
{
    //if((st.st_mode&S_IFMT)==S_IFDIR)
    FILE *fd;
    long file_length=0;
    char buf[BUFFER_SIZE];
    size_t r=0;
    long t;

    fd=fopen(tmp->filepath,"r");
    if(fd==NULL)
    {
        perror("! send_file/fopen error\n");
        exit(1);
    }
    fseek(fd,0,SEEK_END);
    file_length=ftell(fd);
    rewind(fd);

    //设置文件当前指针,为上次没有读完的
    if(tmp->slen>0)
        fseek(fd, tmp->slen, SEEK_SET);
    else
        send_headers(client_fd);
    memset(buf, 0,BUFFER_SIZE);
    r=fread(buf, sizeof(char), BUFFER_SIZE,fd);
    while(1)
    {
        t=send(client_fd,buf,r,0);
        if(t<0)
        {
            if(errno==EAGAIN)
            {
                TIP printf("! EAGAIN");
                return 1;
            }
            else
            {
                printf("! Send Error:");
                exit(1);
            }
        }
        tmp->slen+=t;
        memset(buf, 0,BUFFER_SIZE);
        r=fread(buf, sizeof(char), BUFFER_SIZE,fd);
        if(r==0)
        return 0;
    }
}


void send_not_found(int client)
{
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
    int rcode;
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
    for (; ;)
    {
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
                    //ET until get the errno=EAGAIN
                    while((client_fd= accept(httpd, (struct sockaddr *)&client_addr,&socket_len))>0)
                    {
                        printf("> SOCKET[%d] accept:%s\n",client_fd,inet_ntoa(client_addr.sin_addr));
                        set_nonblocking(client_fd);
                        ev.data.fd = client_fd;
                        ev.events = EPOLLIN|EPOLLET;
                        s=epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                        if (s == -1)
                            epoll_close(epoll_fd,events[i].data.fd,&ev);
                        SocketNode *tmp=new_socket_node();
                        tmp->client_fd=client_fd;
                        add_socket_node(tmp);
                    }
                    if(errno==EAGAIN)
                        TIP printf("! accept EAGAIN\n");
                        //printf("! EAGAIN try again\n");
                    else
                        perror("! Accept Error:");
                    continue;
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
                rcode=send_response(events[i].data.fd);
                if(0==rcode)
                    epoll_close(epoll_fd,events[i].data.fd,&ev);

            }
            else
            {
                printf("uncatched.");
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
    printf("> Socket[%d] close.\n",fd);
}

/*
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
