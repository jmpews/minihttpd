//
// Created by jmpews on 16/7/11.
//

#ifndef HTTPDTMP_UTILS_H
#define HTTPDTMP_UTILS_H

#include <sys/fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/errno.h>
#include <ctype.h>
#include <sys/stat.h>

#define IO_ERROR -1

#define IO_EAGAIN EAGAIN
#define IO_DONE 0
#define IO_LINE_DONE 1
#define IO_LINE_YET 2

#define M_GET 1
#define M_POST 2
#define M_ERROR -1

#define R_HEADER_INIT 0
#define R_HEADER_START 1
#define R_HEADER_BODY 2
#define R_BODY 3
#define R_RESPONSE 4
#define RESPONSE 5

#define is_space(x) isspace((int)(x))

#endif //HTTPDTMP_UTILS_H
