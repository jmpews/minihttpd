//
// Created by jmpews on 16/7/11.
//

#ifndef HTTPDTMP_TYPEDATA_H
#define HTTPDTMP_TYPEDATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<regex.h>

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
    long read_cache_len;     //缓存的内容长度
    char *header_dump;      //缓存请求头
    int header_dump_len;    //请求头的长度
    char method;            //请求方法
    char *request_path;     //请求路径
    char *tmp_file_path;    //临时文件
    long body_len;          //请求的body长度
} Req;

/* 响应结构体 */
typedef struct {
    long response_cache_len;    //响应内容长度
    char *response_path;        //响应文件路径
} Resp;

/* watcher */
typedef struct wt {
    void *watcher_cb;
    int events;
    struct wt *next;
}watcher;

// 前向声明
struct sn;
struct si;
typedef int (*RequestHandler)(struct sn *, struct si *);

/* 正则匹配 */
typedef struct {
    int nm;             //路由中变量个数
    char *pattern;      //路由正则
}RegexRoute;

/* 路由handler */
typedef struct routehandler{
    RegexRoute regexroute;
    int reqstat;                //routehandler触发状态，在R_HEADER_START、R_HEADER_BODY、R_BODY哪个状态触发
    RequestHandler func;
    /*char *(*requesthandler)(SocketNode *);*/
} RouteHandler;


/* socket连接节点 */
typedef struct sn {
    int client_fd;                      //socket连接描述符
    char IO_STATUS;                     //socket状态
    Req request;                        //socket对应的请求
    Resp response;                      //socket对应的响应
    watcher *w;
    struct routehandler *handler;       //路由
    regmatch_t *reg;                    //正则结果
    struct sn *next;
    int events;
} SocketNode;

SocketNode *new_socket_node(int client_fd); //新建一个socket节点

SocketNode *find_socket_node(SocketNode *head, int client_fd); //查找特定描述符节点

void add_socket_node(SocketNode *head, SocketNode *client); //添加socket节点,在head之后

void free_socket_node(SocketNode *head, int client_fd); //释放socket节点

int watcher_add(SocketNode *client_sock, void *watcher_cb, int stat);
int watcher_del(SocketNode *client_sock, void *watcher_cb);


/* 服务器信息 */
typedef struct si{
    char ip[20];                //服务器IP
    int port;                   //服务器端口
    int fd;                     //服务端socket_fd
    char *rootpath;             //服务器根目录
    char *uploadpath;           //服务器上传目录
    char *domain;               //服务器域名
    SocketNode *head_node;      //client-socket链表头结点
    ListNode *head_route;       //路由规则头结点
}ServerInfo;

RouteHandler *get_route_handler(ListNode *head_route, SocketNode *client_sock); //根据SocketNode中的request_path返回路由处理
RouteHandler *get_route_handler_with_reqstat(ListNode *head_route, SocketNode *client_sock, int reqstat);   //根据request_path和请求状态返回路由处理
RouteHandler *new_route_handler(char *pattern, int nm, RequestHandler func, int reqstat); //新建路由 pattern: 路由正则, nm: 正则中变量个数, func: 路由处理函数, reqstat: 路由函数在请求哪个阶段开始处理

extern char *root_path, *upload_path;
#endif //HTTPDTMP_TYPEDATA_H
