//
// Created by jmpews on 16/7/11.
//

#ifndef HTTPDTMP_TYPEDATA_H
#define HTTPDTMP_TYPEDATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define free_buf(buf) if(buf){free(buf);buf=NULL;}

//********************通用性链表**********************

typedef void ElemType;

typedef struct node {
    ElemType *data;
    struct node *next;
} ListNode;


typedef int (*FindElemFunc)(ElemType *, ElemType *key);

void list_append(ListNode *, ElemType *); //添加节点

ElemType *list_get_by_func(ListNode *, FindElemFunc, ElemType *); //通过自定义函数来查找节点

ListNode *new_list_node(); //新建一个链表节点

//******************* socket节点 ***********************

/* 请求结构体 */
typedef struct {
    char *read_cache;       //缓存读取内容
    int read_cache_len;     //缓存的内容长度
    char *header_dump;      //缓存请求头
    int header_dump_len;    //请求头的长度
    char method;            //请求方法
    char *request_path;     //请求路径
    long body_len;          //请求的body长度
} Req;

/* 响应结构体 */
typedef struct {
    long response_cache_len;    //响应内容长度
    char *response_path;        //响应文件路径
} Resp;

/* 路由handler */
typedef int (*RequestHandler)(struct sn *, struct si *);

typedef struct routehandler{
    char urlstring[64];
    int reqstat; //routehandler触发状态，在R_HEADER_START、R_HEADER_BODY、R_BODY哪个状态触发
    RequestHandler func;
    //char *(*requesthandler)(SocketNode *);
} RouteHandler;


/* socket连接节点 */
typedef struct sn {
    int client_fd;          //socket连接描述符
    char IO_STATUS;         //socket状态
    Req request;            //socket对应的请求
    Resp response;          //socket对应的响应
    struct routehandler *handler;
    struct sn *next;
} SocketNode;

SocketNode *new_socket_node(int client_fd); //新建一个socket节点

SocketNode *find_socket_node(SocketNode *head, int client_fd); //查找特定描述符节点

void add_socket_node(SocketNode *head, SocketNode *client); //添加socket节点,在head之后

void free_socket_node(SocketNode *head, int client_fd); //释放socket节点


//********************服务器信息**********************

typedef struct si{
    char ip[20];
    int port;
    int fd;
    char rootpath[256];
    SocketNode *head_node;
    ListNode * head_route;
}ServerInfo;

// 返回一个函数指针,该函数返回接受SocketNode * 参数,返回char *
void init_route_handler(ServerInfo *httpd);
RouteHandler *get_route_handler(ListNode *head_route, char *key);
RouteHandler *get_route_handler_with_reqstat(ListNode *head_route, char *key, int reqstat);
RouteHandler *new_route_handler(char *urlstring, RequestHandler func, int reqstat);
#endif //HTTPDTMP_TYPEDATA_H
