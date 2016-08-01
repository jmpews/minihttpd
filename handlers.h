//
//  handlers.h
//  jmp2httpd
//
//  Created by jmpews on 16/8/1.
//  Copyright © 2016年 jmpews. All rights reserved.
//

#ifndef handlers_h
#define handlers_h

#include "typedata.h"
#include "utils.h"
#include "loop.h"

//extern int send_file(int client_fd, char *path, long *start);
//extern void send_404(int client_fd);
//extern void send_data(int client, char *data);
int accept_handler(ServerInfo *httpd);
void init_route_handler(ServerInfo *httpd);

#endif /* handlers_h */
