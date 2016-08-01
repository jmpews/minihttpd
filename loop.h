//
//  loop.h
//  jmp2httpd
//
//  Created by jmpews on 16/7/31.
//  Copyright © 2016年 jmpews. All rights reserved.
//

#ifndef loop_h
#define loop_h
#include "typedata.h"
#include "loop.h"
#include <arpa/inet.h>

#define IO_READ 1
#define IO_WRITE 2


typedef struct loop {
    int *t;
}LP;


int accept_handler(ServerInfo *httpd);
void select_loop(ServerInfo *httpd);
#endif /* loop_h */
