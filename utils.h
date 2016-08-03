//
// Created by jmpews on 16/7/11.
//

#ifndef HTTPDTMP_UTILS_H
#define HTTPDTMP_UTILS_H

#include <sys/fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include "typedata.h"

#define IO_ERROR -1

#define IO_LINE_DONE 1              // read a line done
#define IO_LINE_NOT_DONE 2          // not done
#define IO_DONE_R 3                 //request read IO_DONE
#define IO_DONE_W 4                 //response write IO_DONE
#define NO_HANDLER 5                //not found match HANDLER
#define IO_EAGAIN_R EAGAIN          //request read IO_EAGAIN
#define IO_EAGAIN_W (EAGAIN+1)      //response write IO_EAGAIN

#define M_GET 1
#define M_POST 2
#define M_ERROR -1

#define R_HEADER_INIT 0             //request state
#define R_HEADER_START 1
#define R_HEADER_BODY 2
#define R_BODY 3
#define R_RESPONSE 4
#define RESPONSE 5

#define is_space(x) isspace((int)(x))

void set_nonblocking(int sockfd);
ServerInfo *startup(int *port, char *root_path, char *upload_path, char *domin);
int handle_request(SocketNode *client_sock, ServerInfo *httpd);
int handle_response(SocketNode *client_sock, ServerInfo *httpd);
char *new_tmp_file(ServerInfo *httpd, char *optional);
int read_tmp_file(int client_fd, char *path, int *start, int bodylen);
int send_file(int client_fd, char *path, int *start);
void send_404(int client_fd);
void send_data(int client, char *data);
int handle_response_with_reqstat(SocketNode *client_sock, ServerInfo *httpd, int reqstat);
int handle_response_with_default_handler(SocketNode *client_sock, ServerInfo *httpd);
int handle_response_with_handler(SocketNode *client_sock, ServerInfo *httpd);
#endif //HTTPDTMP_UTILS_H
