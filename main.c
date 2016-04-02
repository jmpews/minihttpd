//
//  main.c
//  jmp2httpd
//
//  Created by jmpews on 15/12/12.
//  Copyright © 2015年 jmpews. All rights reserved.
//

#include <stdio.h>
#include "main.h"
#include <libgen.h>
int main(int argc, const char * argv[]) {
    
    /* struct sockaddr_in server_addr = {0}; */
    int port;
    INT_32 httpd;
    if( argc <= 2)
    {
        printf("usage: %s need port\n",basename(argv[0]));
    }
    port=atoi(argv[1]);
    init_route();
    if((httpd=startup(&port))==-1)
    {
        printf("! init socket() error.");
        exit(1);
    }
    
    printf("> start listening at %d\n",port);
    
    select_loop(httpd);
}
