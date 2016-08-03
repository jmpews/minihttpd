 /*
 * =====================================================================================
 *
 *  Filename:       typedata.c
 *
 *  Description:    类型描述文件
 *
 *  Version:        1.0
 *  Created:        2016/07/11 15时01分42秒
 *  Compiler:       gcc
 *
 *  Author:         jmpews (jmpews.github.io), jmpews@gmail.com
 *
 * =====================================================================================
 */

#include "typedata.h"


// 链表添加节点
void list_append(ListNode *head, ElemType *elem) {
    ListNode *node;
    node = new_list_node();
    node->data = elem;
    node->next = head->next;
    head->next = node;
}


// 新建链表节点
ListNode *new_list_node() {
    ListNode *tmp;
    tmp = (ListNode *) malloc(sizeof(ListNode));
    if (tmp == NULL) {
        printf("ERROR-[new_list_node] : memory malloc return NULL\n");
        exit(1);
    }
    tmp->data = NULL;
    tmp->next = NULL;
    return tmp;
}


// 新建socketnode节点
SocketNode *new_socket_node(int fd) {
    SocketNode *tmp;
    tmp = (SocketNode *) malloc(sizeof(SocketNode));
    if (tmp == NULL) {
        printf("ERROR-[new_socket_node] : memory malloc return NULL\n");
        exit(1);
    }
    memset(tmp, 0, sizeof(SocketNode));
    tmp->client_fd=fd;
    tmp->request.read_cache = NULL;
    tmp->request.header_dump = NULL;
    return tmp;
}

SocketNode *find_socket_node(SocketNode *head, int client_fd) {
    SocketNode *tmp = head;
    if (head == NULL) {
        printf("ERROR-[find_socket_node]: socket-node-head is NULL when find [%d-socket-node]\n", client_fd);
        exit(1);
    }
    do {
        if (tmp->client_fd == client_fd)
            return tmp;
        tmp = tmp->next;
    } while (tmp);
    return NULL;
}

// 添加新节点，新节点添加在head之后
void add_socket_node(SocketNode *head, SocketNode *client) {
    if (head == NULL) {
        printf("ERROR-[add_socket_node]: socket-node-head is NULL when add [%d-socket-node]\n", client->client_fd);
        exit(1);
    }
    client->next = head->next;
    head->next = client;
}

// 释放socketnode节点
void free_socket_node(SocketNode *head, int client_fd) {
    SocketNode *tmp = head;
    SocketNode *tmp2free = NULL;
    // 头结点是服务器listen
    if (head == NULL) {
        printf("ERROR-[free_socket_node]: socket-node-head is NULL when free [%d-socket-node]\n", client_fd);
        exit(1);
    }

    //找到需要释放节点的前一个节点
    while ((tmp->next) != NULL) {
        if (tmp->next->client_fd == client_fd)
            break;
        tmp = tmp->next;
    }

    //没找到node
    if ((tmp->next) == NULL) {
        printf("ERROR-[free_socket_node]: heade_node is empty when free [socket-%d]\n", client_fd);
        exit(1);
    }
    tmp2free = tmp->next;
    tmp->next = tmp->next->next;
    
    printf("> [socket-%d] free.\n", client_fd);
    free_buf(tmp2free->request.read_cache);
    free_buf(tmp2free->request.header_dump);
    free_buf(tmp2free->request.request_path);
    free_buf(tmp2free->request.tmp_file_path);
    free_buf(tmp2free->response.response_path);
    free_buf(tmp2free->reg);
    free_buf(tmp2free);
}

// watcher_add
int watcher_add(SocketNode *client_sock, void *watcher_cb, int stat) {
    watcher *tmp = (watcher *)malloc(sizeof(watcher));
    memset(tmp, 0, sizeof(watcher));
    tmp->watcher_cb = watcher_cb;
    tmp->events |= stat;
    if(client_sock->w != NULL) {
        tmp->next = client_sock->w;
        client_sock->w = tmp;
    } else {
        client_sock->w = tmp;
    }
    return 0;
}

// watcher_delete
int watcher_del(SocketNode *client_sock, void *watcher_cb) {
    watcher *nd;
    watcher *tmp = client_sock->w;
    if(tmp->watcher_cb == watcher_cb) {
        free(tmp);
        client_sock->w = NULL;
        return 0;
    }
    while (tmp->next) {
        if(tmp->next->watcher_cb == watcher_cb) {
            nd = tmp->next;
            tmp->next = nd->next;
            free(nd);
            return 0;
        }
    }
    return 1;
}

// 采用正则匹配路由
int regex_route(RegexRoute rr, SocketNode *client_sock) {
    regex_t reg;
    int err;
    char errbuf[1024];
    client_sock->reg = (regmatch_t *)malloc(sizeof(regmatch_t) * rr.nm);
    // 编译正则
    if(regcomp(&reg,rr.pattern,REG_EXTENDED) < 0){
        regerror(err,&reg,errbuf,sizeof(errbuf));
        printf("ERROR-[regex_route]: regex_route error of%s\n",errbuf);
        exit(-1);
    }
    // 匹配正则，如果有变量保存到client_sock->reg
    err = regexec(&reg, client_sock->request.request_path, rr.nm, client_sock->reg,0);
    if(err == REG_NOMATCH){
        return 0;
        
    }else if(err){
        regerror(err,&reg,errbuf,sizeof(errbuf));
        printf("ERROR: regex_route of err:%s\n",errbuf);
        exit(-1);
    }
    return 1;
}

// 根据请求路径和路由正则返回handler处理函数
RouteHandler *get_route_handler(ListNode *head_route, SocketNode *client_sock) {
    ListNode *tmp = head_route->next;
    RouteHandler *uh= (RouteHandler *)head_route->data;
    while(tmp) {
        uh = (RouteHandler *)tmp->data;
        if (regex_route(uh->regexroute, client_sock)) {
            printf("> [socket-%d] match route %s.\n", client_sock->client_fd, uh->regexroute.pattern);
            return uh;
        }
        tmp = tmp->next;
    }
    return NULL;
}

//根据请求路径和路由正则还有reqstat(哪个阶段开始处理请求)
RouteHandler *get_route_handler_with_reqstat(ListNode *head_route, SocketNode *client_sock, int reqstat) {
    ListNode *tmp = head_route->next;
    RouteHandler *uh;
    while(tmp) {
        uh = (RouteHandler *)tmp->data;
        if ((uh->reqstat == reqstat) && (regex_route(uh->regexroute, client_sock))) {
            printf("> [socket-%d] match route %s.\n", client_sock->client_fd, uh->regexroute.pattern);
            return uh;
        }
        tmp = tmp->next;
    }
    return NULL;
}

// 设置路由正则
void set_regex_route(char *pattern, int nm, RouteHandler *rh) {
    rh->regexroute.pattern = (char *)malloc(sizeof(char) * strlen(pattern));
    strcpy(rh->regexroute.pattern, pattern);
    rh->regexroute.nm = nm;
}

// 新建路由handler
RouteHandler *new_route_handler(char *pattern, int nm, RequestHandler func, int reqstat) {
    RouteHandler *tmp = (RouteHandler *) malloc(sizeof(RouteHandler));
    set_regex_route(pattern, nm+1, tmp);
    tmp->func = func;
    tmp->reqstat = reqstat;
    return tmp;
}