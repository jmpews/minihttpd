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

/*
 * ===  FUNCTION  ======================================================================
 *  Name:  list_append
 *  Description:  向通用性链表追加节点, 新节点追加在head之后
 *  @param head 链表头节点
 *  @param elem 元素节点
 * =====================================================================================
 */

void list_append(ListNode *head, ElemType *elem) {
    ListNode *node;
    node = new_list_node();
    node->data = elem;
    node->next = head->next;
    head->next = node;
}


/*
 * ===  FUNCTION  ======================================================================
 *  Name:  list_get_by_func
 *  Description:  通过自定义函数查找节点
 * =====================================================================================
 */

ElemType *list_get_by_func(ListNode *head, FindElemFunc func, void *key) {
    ListNode *tmp = head->next;
    if (tmp == NULL)
        return NULL;
    while (tmp) {
        if (func(tmp->data, key))
            return tmp->data;
        tmp = tmp->next;
    }
    return NULL;
}

ListNode *new_list_node() {
    ListNode *tmp;
    tmp = (ListNode *) malloc(sizeof(ListNode));
    if (tmp == NULL) {
        printf("ERROR : memory malloc return NULL");
        exit(1);
    }
    tmp->data = NULL;
    tmp->next = NULL;
    return tmp;
}


SocketNode *new_socket_node(int fd) {
    SocketNode *tmp;
    tmp = (SocketNode *) malloc(sizeof(SocketNode));
    if (tmp == NULL) {
        printf("ERROR : memory malloc return NULL");
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
        printf("ERROR: socket-node-head is NULL when find [%d-socket-node]\n", client_fd);
        exit(1);
    }
    do {
        if (tmp->client_fd == client_fd)
            return tmp;
    } while (tmp = tmp->next);
    return NULL;
}

void add_socket_node(SocketNode *head, SocketNode *client) {
    if (head == NULL) {
        printf("ERROR: socket-node-head is NULL when add [%d-socket-node]\n", client->client_fd);
        exit(1);
    }
    client->next = head->next;
    head->next = client;
}

void free_socket_node(SocketNode *head, int client_fd) {
    SocketNode *tmp = head;
    SocketNode *tmp2free = NULL;
    //空链表
    if (head == NULL) {
        printf("ERROR: socket-node-head is NULL when free [%d-socket-node]\n", client_fd);
        exit(1);
    }

    //TODO: 这里到底是否需要判断头结点
    if (head->client_fd == client_fd) {
        head = NULL;
        free_buf(head->request.read_cache);
        free_buf(head->request.header_dump);
        free_buf(head->request.request_path);
        free_buf(head->response.response_path);
        free_buf(head);
//        close(client_fd);
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
        printf("ERROR: socket-node-list is empty when free [%d-socket-node]\n", client_fd);
        exit(1);
    }

    tmp2free = tmp->next;
    tmp->next = tmp->next->next;
    printf("DEBUG: free [socket-node-%d],detail:{request-path:%s}\n", client_fd, tmp2free->request.request_path);
    free_buf(tmp2free->request.read_cache);
    free_buf(tmp2free->request.header_dump);
    free_buf(tmp2free->request.request_path);
    free_buf(tmp2free->response.response_path);
    free_buf(tmp2free);

//    close(client_fd);
//    TIP printf("> SOCKET[%d] ready close.\n", client_fd);
}

char *route_func1(SocketNode *tmp) {
    char *str;
    char *s = "route.1:hello world\n\0";
    str = (char *) malloc(1024 * sizeof(char));
    strcpy(str, s);
    str[strlen(s)] = '\0';
    return str;
}

char *route_func2(SocketNode *tmp) {
    char *str;
    char *s = "route.2:hello world\n\0";
    str = (char *) malloc(1024 * sizeof(char));
    strcpy(str, s);
    str[strlen(s)] = '\0';
    return str;
}

char *route_func3(SocketNode *tmp) {
    return tmp->request.header_dump;
}

int find_route(ElemType *data, void *key) {
    if (!strcasecmp((char *) key, ((UrlRoute *) data)->route)) {
        printf("匹配到route:%s", (char *) key);
        return 1;
    }
    return 0;
}

UrlRoute *new_route_node(char *route_str, RouteFunc func) {
    UrlRoute *tmp = (UrlRoute *) malloc(sizeof(UrlRoute));
    strcpy(tmp->route, route_str);
    tmp->func = func;
    return tmp;
}

void init_route(ServerInfo *httpd) {
    httpd->head_route = new_list_node();
    list_append(httpd->head_route, new_route_node("/route1", route_func1));
    list_append(httpd->head_route, new_route_node("/route2", route_func2));
    list_append(httpd->head_route, new_route_node("/echo", route_func3));
}

char *handle_route(ServerInfo *httpd, SocketNode *client_sock, char *route_key) {
    char *resp;
    ElemType *data;
    data = list_get_by_func(httpd->head_route, find_route, route_key);
    if (data) {
        resp = ((UrlRoute *) data)->func(client_sock);
        return resp;
    }
    else {
        return NULL;
    }
}