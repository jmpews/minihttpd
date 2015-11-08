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

#ifndef sockets_h
#define sockets_h

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INT_32 int
#define BACKLOG 20
#define MAX_CLIENTS 1024
#define PORT 8080
#define MAX_BUFFER_SIZE 1024
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
INT_32 startup(u_short *port);
void send_response(int client_fd);

#endif /* sockets_h */


struct clinfo{
    int client_fd;
    char req_file[100];
    int filed;
};

INT_32 set_nonblocking(INT_32 sockfd)
{
    INT_32 opts;
    opts = fcntl(sockfd, F_GETFL);
    if(opts < 0) {
        perror("fcntl: F_GETFL");
        return -1;
    }
    
    opts = opts | O_NONBLOCK;
    if( fcntl(sockfd, F_SETFL, opts) < 0 ) {
        perror("fcntl: F_SETFL");
        return -1;
    }
    
    printf("set socket %d to non blocking successfully.\n", sockfd);
    
    return 0;
}



INT_32 startup(u_short *port)
{
    int httpd=0;
    struct sockaddr_in server_addr;
    if((httpd=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        perror("httpd start error.");
        exit(1);
    }
    
    INT_32 opt = SO_REUSEADDR;
    if(setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("httpd reuseaddr error.");
        exit(1);
    }
    
    //set no-blocking
    set_nonblocking(httpd);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(httpd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("httpd bind error.");
        return -1;
    }
    
    if(listen(httpd, BACKLOG) == -1)
    {
        perror("httpd listen error.");
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
    char *wwwpath;
    int i=0,j=0;
    char *query_string = NULL;
    wwwpath=clients[client_fd]->req_file;
    r=get_line(client_fd, buf, sizeof(buf));
    //read request method
    while (!(is_space(buf[j]))&&i<sizeof(buf))
    {
        method[i]=buf[j];
        i++;j++;
    }
    method[i]='\0';
    if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))
    {
        perror("request method not support.");
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
    
    printf("SOCKET-%d USER HEADER:",client_fd);
    while ((r>0)&&strcmp("\n", buf))
    {
        r=get_line(client_fd, buf, sizeof(buf));
        printf("%s",buf);
    }
    
    sprintf(wwwpath, "%s/htdocs%s",rootpath,url);
    //sprintf(wwwpath, "/Users/jmpews/Desktop/netcore/netcore/htdocs/index.html");
    clients[client_fd]->filed=1;
    if (wwwpath[strlen(wwwpath)-1]=='/')
        strcat(wwwpath, "index.html");
    if (stat(wwwpath,&st)==-1)
        clients[client_fd]->filed=0;
    else
        if((st.st_mode&S_IFMT)==S_IFDIR)
        {
            strcat(wwwpath, "/index.html");
            if((st.st_mode&S_IFMT)!=S_IFREG)
                clients[client_fd]->filed=0;
        }
    return 1;
}

void send_response(int client_fd)
{
    if(clients[client_fd]->filed==1)
        serve_file(client_fd, clients[client_fd]->req_file);
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



INT_32 read_data(INT_32 fd,char *buffer)
{
    char tmp[1024];
    INT_32 r=0;
    INT_32 n=0;
    memset(&tmp, 0, sizeof(tmp));
    do {
        memcpy(buffer,tmp,r*sizeof(char));
        r = recv(fd, tmp+n*sizeof(char), MAX_BUFFER_SIZE*sizeof(char), 0);
        n+=r;
    } while (r>0);
    if(n>0) {
        //read data,首先判断可否读取，如果可以读取，那么一直持续读取，直到result==-1
        printf("RESULT:%s\n",buffer);
        return 1;
    } else if (n == 0) {
        //close socket
        close(fd);
        printf("close connect.\n");
        return -1;
    }
    return -1;
}

void send_data(INT_32 fd,const char *buffer)
{
    INT_32 len=strlen(buffer);
    INT_32 r;
    INT_32 n=0;
    do {
        r = send(fd, buffer+n, len*sizeof(char), 0);
        if (r > 0)
            n += r;
        else if (r < 0)
            break;
    } while (n<len);
}

