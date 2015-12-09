#define LOG_INFO(x) printf("[LOG-INFO]--%s\n", x);fflush(stdout);
//url 路由匹配,不打算用链表存储因为路由表不是动态增加的
typedef struct urlroute{
    char route[64];
    char *(*func)(SocketNode *);
} URL_ROUTE;

URL_ROUTE route[3];

void set_url_route(URL_ROUTE *route,char *url_route,char *(*func)(SocketNode *)){
    strcpy(route.route,url_route);
    route.func=func;
}

char (*get_route_func(URL_ROUTE *route,char *url_route,int count))(SocketNode *){
    int i=0;
    for (int i = 0; i < count; ++i)
    {
        if(!strcasecmp(route[i].route,url_route))
        {
            return route[i].func;
        }
    }
}
