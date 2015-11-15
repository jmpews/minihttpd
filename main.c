//
//  main.c
//  netcore
//
//  Created by jmpews on 15/11/6.
//  Copyright © 2015年 jmpews. All rights reserved.
//

#include <sys/select.h>
#include "main.h"

INT_32 main(INT_32 argc, const char * argv[]) {

    /* struct sockaddr_in server_addr = {0}; */
    int port=7777;
    INT_32 httpd;
    if((httpd=startup(&port))==-1)
    {
        printf("! init socket() error.");
        exit(1);
    }

    printf("> start listening at %d\n",port);

    start_epoll_loop(httpd);
}
