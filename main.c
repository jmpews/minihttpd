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
    INT_32 listen_fd;
    INT_32 connect_fd;
    INT_32 retcode;
    int port=7777;
    INT_32 checks[MAX_CLIENTS];
    INT_32 result;
    INT_32 maxfd;
    INT_32 i;
    /* struct sockaddr_in server_addr = {0}; */
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    
    if((listen_fd=startup(&port))==-1)
    {
        perror("! init socket() error.");
        exit(1);
    }
    printf("> start listening at %d\n",port);
    socklen_t addr_len=sizeof(struct sockaddr_in);
    memset(checks, 0, sizeof(checks));
    fd_set read_fds;
    fd_set write_fds;
    fd_set exception_fds;
    fd_set tmp_read_fds;
    fd_set tmp_write_fds;
    fd_set tmp_exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&exception_fds);
    
    /* wait for new connect */
    FD_SET(listen_fd,&read_fds);
    checks[listen_fd]=1;
    
    struct timeval tv;
    tv.tv_sec=5;
    tv.tv_usec=0;
    
    while (1) {
        tmp_read_fds=read_fds;
        tmp_write_fds=write_fds;
        tmp_exception_fds=exception_fds;
        maxfd=0;
        
        for (i=0; i<MAX_CLIENTS; i++)
        {
            if (checks[i])
                if (maxfd<i)
                    maxfd=i;
        }
        
        retcode=select(maxfd+1, &tmp_read_fds, &tmp_write_fds, &tmp_exception_fds, &tv);
        if (retcode<0)
            perror("select() error.");
        else if(retcode==0)
        {
            continue;
        } else {
            // new connect
            if (FD_ISSET(listen_fd,&tmp_read_fds)) {
                connect_fd=accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
                struct clinfo *cli=(struct clinfo*)malloc(sizeof(struct clinfo));
                cli->client_fd=connect_fd;
                clients[connect_fd]=cli;
                
                if (connect_fd<0)
                    perror("! client connect error.");
                else
                {
                    printf("> connect %d comming.\n",connect_fd);
                    set_nonblocking(connect_fd);
                    FD_SET(connect_fd,&read_fds);
                    checks[connect_fd]=1;
                    maxfd=(maxfd < connect_fd)?connect_fd:maxfd;
                }
            } else{
                for(i=listen_fd+1;i<=maxfd;i++){
                    if(checks[i])
                    {
                        if(FD_ISSET(i,&tmp_read_fds))
                        {
                            //read data or close connect
                            result=accept_request(i);
                            if(result==1)
                            {
                                FD_CLR(i,&read_fds);
                                FD_SET(i,&write_fds);
                            } else if (result == 0)
                                FD_CLR(i, &read_fds);
                        }
                        else if(FD_ISSET(i,&tmp_write_fds))
                        {
                            //send data
                            send_response(i);
                            close(i);
                            FD_CLR(i,&write_fds);
                            checks[i]=0;
                            free(clients[i]);
                        }
                    }
                }
            }
        }
    }
}
