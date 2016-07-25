//
// Created by jmpews on 16/7/11.
//

#include "utils.h"
#include "typedata.h"

extern int debug_header, debug_body, debug_tips;

//*****************************************  服务器初始化模块  ************************************

void set_nonblocking(int sockfd) {
    int opts;
    opts = fcntl(sockfd, F_GETFL);
    if (opts < 0) {
        printf("ERROR: fcntl F_GETFL\n");
        exit(1);
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0) {
        printf("ERROR: fcntl F_SETFL\n");
        exit(1);
    }
    if(debug_tips)
        printf("> socket-[%d] non-blocking.\n", sockfd);
}

ServerInfo *startup(int *port) {
    int fd = 0;
    struct sockaddr_in server_addr;
    ServerInfo *httpd;
    httpd = (ServerInfo *) malloc(sizeof(ServerInfo));
    memset(httpd, 0, sizeof(ServerInfo));
    getcwd(httpd->rootpath, sizeof(httpd->rootpath));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("ERROR: httpd start error.");
        exit(1);
    }

    int opt = SO_REUSEADDR;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        printf("ERRPR: socket start error of reuse error.");
        exit(1);
    }

    //设置非阻塞
    set_nonblocking(fd);

    //新建头结点
    httpd->head_node = new_socket_node(fd);
    httpd->fd = fd;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1) {
        printf("ERRPR: http start error of bind error.\n");
        exit(1);
    }

    if (listen(fd, 1024) == -1) {
        printf("ERRPR: http start error of listen error.\n");
        exit(1);
    }

    return httpd;
}

//*****************************************  Request模块  ************************************

/*
 * 读到buf,返回状态码
 * 1. 读取状态码(失败、成功、EAGAIN)
 * 2. 读取内容长度
 * 3. 一行数据有没有读取完毕
*/
int read_line(int sock, char *buffer, int size, int *len) {
    char c = '\0';
    int r = 0, t = 0;
    *len = 0;
    //buf[BUF_SIZE-1] must be '\0'
    while ((t < size - 1) && (c != '\n')) {
        r = recv(sock, &c, 1, 0);
        if (r > 0) {
            // 判断下一个符号是否是\r，如果是则表明为\r\n结束符
            if (c == '\r') {
                // MSG_PEEK:从缓冲区copy数据，并不删除数据，如果符合再次读取数据
                r = recv(sock, &c, 1, MSG_PEEK);
                if (r > 0 && c == '\n')
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buffer[t] = c;
            t++;
        }
        else
            break;
    }

    buffer[t++] = '\0';
    *len = t;

    if (r < 0) {
        // 缓冲区为空,读取失败
        if (errno == EAGAIN) {
            return IO_EAGAIN_R;
        }
        else {
            printf("ERROR: error at read_line");
            return IO_ERROR;
        }
    }
    else if (c == '\n')
        return IO_LINE_DONE;
	else
		return IO_LINE_NOT_DONE;
}

//读取一行无论数据有多长
/*
 * @malloc_buffer 读取内容
 *
 */
int read_line_more(int client_fd, char **malloc_buffer, int *len) {
    int r;
    int n = 0;
    int t;
    char *temp_buffer = NULL;
    int buffer_size = 1024;
    char buf[buffer_size];

    memset(buf, 0, buffer_size);
    *len = 0;

    r = read_line(client_fd, buf, buffer_size, &t);

    /*
     * 读取流程:
     * 读取一行(buf_size=1024)
     * 拷贝到buf
     * >进行状态判断
     * 1. 最后字符!='\n',且读取状态为IO_LINE_DONE
     *      重新读取
     * 2. 读取错误IO_EAGAIN || IO_ERROR
     *      返回buf+状态码
     * 3. 最后字符为'\n'
     *      读取完毕，返回返回buf+状态码
     */
    while (1 && t > 1) {
        if (!temp_buffer)
            temp_buffer = (char *) malloc(sizeof(char) * t);
        else
            temp_buffer = (char *) realloc(temp_buffer, (n + t) * sizeof(char));
        memcpy(temp_buffer + n, buf, t);
        n += t;
		// 一行没有读完,double check，其实多余
        if (buf[t - 1 - 1] != '\n' && r == IO_LINE_NOT_DONE) {
            r = read_line(client_fd, buf, buffer_size, &t);
        }
		// 一行读完
        else if (buf[t - 1 - 1] == '\n' && r == IO_LINE_DONE) {
            *len = n;
            *malloc_buffer = temp_buffer;
            return r;
        }
		// 需要重新挂起等待
        else if (r == IO_EAGAIN_R) {
            *len = n;
            *malloc_buffer = temp_buffer;
            return r;
        }
		else {
			printf("ERROR: unknown error at read_line_more\n");
			exit(1);
		}
    }
    return r;
}

// 读取到temp目录作为临时文件
int read_tmp_file(int client_fd, char *path, long *start) {
    int r, n;
    FILE *fd;
    struct stat st;
    int buffer_size = 1024*2;
    char buf[buffer_size+1];
    if ((stat(path, &st) != -1) && ((st.st_mode & S_IFMT) == S_IFREG)) {
        fd = fopen(path, "w+");
        if(!fd) {
            printf("ERROR: %s open failed.\n", path);
        }
        while(1) {
            memset(buf, 0, buffer_size+1);
            n = recv(client_fd, buf, buffer_size, 0);
            r = fwrite(buf, sizeof(char), n, fd);
            if (n == -1) {
                if (errno == EAGAIN) {
                    printf("> EAGAIN: read_block:%s\n", path);
                    fclose(fd);
                    return IO_EAGAIN_R;
                } else {
                    printf("ERROR: unknown at read_block.\n");
                    exit(1);
                }
            }
            else if(r != n) {
                printf("ERROR: n!=r at send_file.\n current_cache_length=%ld, r=%d, n=%d", *start, r, n);
                exit(1);
            }
            else {
                if(n != buffer_size) {
                     if (ferror(fd)) {
                        printf("ERROR: %s reading error.", path);
                    }
                    else if (n < buffer_size) {
                        fclose(fd);
                        return IO_DONE_W;
                    }
                }
            }
            *start += n;
        }
    } else {
        printf("ERROR: unknown at read_block\n");
        exit(1);
    }
}

char *new_tmp_file(ServerInfo *httpd) {
    char *tmp_file_path;
    char tmp[128];
    FILE *fd;
    tmp_file_path = (char *)malloc(sizeof(char) * 256);
    memset(tmp_file_path, 0, 256);

    time_t nowtime = time(NULL);
    struct tm *now = localtime(&nowtime);
    sprintf(tmp, "jmp.%d.%d.%d.%d.%d.%d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
    sprintf(tmp_file_path, "%s/upload/%s", httpd->rootpath, tmp);
    fd = fopen(tmp_file_path, "w+");
    if(fd == NULL) {
        printf("ERROR: new file error.\n");
        exit(1);
    }
    fclose(fd);
    return tmp_file_path;
}


int handle_error(int client_fd) {
    char *malloc_buf;
    int len;
    read_line_more(client_fd, &malloc_buf, &len);
    do {
        if (1) //是否打印header
            printf("%s", malloc_buf);
        free_buf(malloc_buf);
    } while (read_line_more(client_fd, &malloc_buf, &len) == IO_LINE_DONE);
    free_buf(malloc_buf);
    return IO_ERROR;
}
void handle_eagain_cache(SocketNode *client_sock, int r, char *malloc_buf, int len)  {
    // 如果read_line_more 读取数据，遇到EAGAIN，需要保存cache，需要加上之前的cache
    // 如果read_line_more 读取数据，遇到IO_LINE_DONE，表明读取完毕，查看之前是否有cache，需要加上之前的cache
    if( (r== IO_EAGAIN_R && len >1) || (r == IO_LINE_DONE && client_sock->request.read_cache > 0)){
        client_sock->request.read_cache = (char *) realloc(client_sock->request.read_cache,
                                                       (client_sock->request.read_cache_len + len));
        memcpy(client_sock->request.read_cache + client_sock->request.read_cache_len, malloc_buf,
           client_sock->request.read_cache_len);
        client_sock->request.read_cache_len += len;
        free(malloc_buf);

    } else {
        client_sock->request.read_cache = malloc_buf;
        client_sock->request.read_cache_len = len;
    }
}

void save_header_dump(SocketNode *client_sock) {
    client_sock->request.header_dump = (char *) realloc(client_sock->request.header_dump,client_sock->request.read_cache_len + client_sock->request.header_dump_len - 1);
    memcpy(client_sock->request.header_dump + client_sock->request.header_dump_len, client_sock->request.read_cache, client_sock->request.read_cache_len - 1);
    client_sock->request.header_dump_len += client_sock->request.read_cache_len - 1;
    free_buf(client_sock->request.read_cache);
    client_sock->request.read_cache_len = 0;
}
//处理请求的第一行,获取请求方法,请求路径
int request_header_start(int client_fd, SocketNode *client_sock) {
    int buffer_size = 1024;
    int r, i, j, len;
    char tmp_buf[1024];
    char *malloc_buf = NULL;

    if(debug_header)
        printf("socket-%d] request-header: \n", client_fd);
    r = read_line_more(client_fd, &malloc_buf, &len);
    if (r == IO_ERROR) {
        printf("ERROR: request_header_start...\n");
        exit(1);
    }
    // read_cache内有上次缓存数据
    // 状态为IO_EAGAIN,表明缓冲区还有数据
    handle_eagain_cache(client_sock, r, malloc_buf, len);
    malloc_buf = client_sock->request.read_cache;
    len = client_sock->request.read_cache_len;
    if (client_sock->IO_STATUS == R_HEADER_START) {
        if (r == IO_EAGAIN_R) {
            return IO_EAGAIN_R;
        }
    }

    if (r == IO_LINE_DONE) {
        // 设置下一个状态
        client_sock->IO_STATUS = R_HEADER_BODY;

        i = j = 0;
        while (!(is_space(malloc_buf[j])) && i < len) {
            tmp_buf[i] = malloc_buf[j];
            i++;
            j++;
        }
        tmp_buf[i] = '\0';
        if (strcasecmp(tmp_buf, "GET") && strcasecmp(tmp_buf, "POST") && strcasecmp(tmp_buf, "PUT")) {
            free_buf(malloc_buf);
            len = 0;
            printf("ERROR: request_header_start error of \n%s", malloc_buf);
            return handle_error(client_fd);
            exit(1);
        }
        if (!strcasecmp(tmp_buf, "GET"))
            client_sock->request.method = M_GET;
        else if ((!strcasecmp(tmp_buf, "POST"))||(!strcasecmp(tmp_buf, "PUT")))
            client_sock->request.method = M_POST;
        else
            client_sock->request.method = M_ERROR;
        while (is_space(malloc_buf[j]) && (j < buffer_size))
            j++;

        //设置请求路径
        i = 0;
        while (!is_space(malloc_buf[j]) && (j < len) && (malloc_buf[i] != '?')) {
            tmp_buf[i] = malloc_buf[j];
            i++;
            j++;
        }
        tmp_buf[i] = '\0';
        client_sock->request.request_path = (char *) malloc((i + 1) * sizeof(char));
        memcpy(client_sock->request.request_path, tmp_buf, i + 1);

        //打印，保存到header_dump
        if (debug_header)
            printf("  %s", malloc_buf);

        save_header_dump(client_sock);

        return IO_DONE_R;
    }
    return IO_ERROR;
}

void handle_header_kv(int client_fd, char *buf, int len, SocketNode *client_sock) {
    char key[64];
    int i;
    for (i = 0; i < len; i++) {
        if (buf[i] == ':')
            break;
    }
    memcpy(key, buf, i);
    key[i] = '\0';
    i += 1;

    if (!strcasecmp(key, "Content-Length")) {
        client_sock->request.body_len = atol(buf + i);
    }
}

/*
 *  请求header的处理流程:
 */
int request_header_body(int client_fd, SocketNode *client_sock) {
    int r, len;
    short end;
    char *malloc_buf = NULL;
    do {
        len = 0;
        end = 0;
        malloc_buf = NULL;
        r = read_line_more(client_fd, &malloc_buf, &len);
        if (r == IO_ERROR) {
            printf("ERROR: request_header_body...\n");
            exit(1);
        }
        // read_cache内有上次缓存数据
        // 状态为IO_EAGAIN,表明缓冲区还有数据
        handle_eagain_cache(client_sock, r, malloc_buf, len);
        malloc_buf = client_sock->request.read_cache;
        len = client_sock->request.read_cache_len;
        if (client_sock->IO_STATUS == R_HEADER_BODY) {
            if (r == IO_EAGAIN_R) {
                return IO_EAGAIN_R;
            }
        }

        if (r == IO_LINE_DONE) {
            handle_header_kv(client_fd, malloc_buf, len, client_sock);
        }
        if (debug_header)
            printf("  %s", malloc_buf);

        if(strcasecmp(malloc_buf, "\n"))
            end = 1;
        // 保存整个header
        save_header_dump(client_sock);
    } while (end && r == IO_LINE_DONE);

    // 设置下一个状态
    client_sock->IO_STATUS = R_BODY;
    return IO_DONE_R;
}


int request_body(int client_fd, SocketNode *client_sock) {
    int buffer_size = 1024;
    int r;
    char *malloc_buf = NULL;
    int len = 0;
    char *buffer;
    if(debug_body)
        printf("[socket-%d] request-body:\n", client_fd);
    //PRINT_LINE_TITLE("header-end");
    // body没有数据
    if (!client_sock->request.body_len) {
        client_sock->IO_STATUS = R_RESPONSE;
        return IO_DONE_R;
    }
    do {
        free_buf(malloc_buf);
        r = read_line_more(client_fd, &malloc_buf, &len);
        if (r == IO_ERROR) {
            printf("ERROR: request_header_body...\n");
            exit(1);
        }
        // read_cache内有上次缓存数据
        // 状态为IO_EAGAIN,表明缓冲区还有数据
        handle_eagain_cache(client_sock, r, malloc_buf, len);
        malloc_buf = client_sock->request.read_cache;
        len = client_sock->request.read_cache_len;
        if (client_sock->IO_STATUS == R_BODY) {
            if (r == IO_EAGAIN_R) {
                return IO_EAGAIN_R;
            }
        }

        if (r == IO_LINE_DONE) {
            // 缓冲区有数据
            if(debug_body)
                printf("  %s", malloc_buf);
            if (len + 5 >= client_sock->request.body_len) {
                break;
            }
        }
    } while (r == IO_LINE_DONE);
    
    free_buf(client_sock->request.read_cache);
    client_sock->request.read_cache_len = 0;
    client_sock->IO_STATUS = R_RESPONSE;
    return IO_DONE_R;
}

// 根据状态机的思路
int handle_request(SocketNode *client_sock, ServerInfo *httpd) {
    int r;
    int client_fd=client_sock->client_fd;
    switch (client_sock->IO_STATUS) {
        case R_HEADER_INIT: {
            printf("\0");
            client_sock->IO_STATUS = R_HEADER_START;
        }
        case R_HEADER_START: {
            // 在header_start就开始使用handler
            r = handle_response_with_reqstat(client_sock, httpd, R_HEADER_START);
            if(r != NO_HANDLER) {
                return r;
            }
            r = request_header_start(client_fd, client_sock);
            if (r == IO_EAGAIN_R) {
                if (client_sock->request.method == M_ERROR)
                    return IO_ERROR;
                return IO_EAGAIN_R;
            }
            else if (r == IO_ERROR)
                return IO_ERROR;
            else if (r == IO_DONE_R) {

            }
            else {
                printf("ERROR: unknown error at R_HEADER_START.\n");
                exit(1);
            }
        }
        case R_HEADER_BODY: {
            // 在handler_key_value的时候
            r = handle_response_with_reqstat(client_sock, httpd, R_HEADER_BODY);
            if(r != NO_HANDLER) {
                return r;
            }
            r = request_header_body(client_fd, client_sock);
            if (r == IO_EAGAIN_R) {
                if (client_sock->request.method == M_ERROR)
                    return IO_ERROR;
                return IO_EAGAIN_R;
            }
            else if (r == IO_ERROR)
                return IO_ERROR;
            else if (r == IO_DONE_R) {

            }
            else {
                printf("ERROR: unknown error at R_HEADER_BODY.\n");
                exit(1);
            }
        }
        case R_BODY: {
            r = handle_response_with_reqstat(client_sock, httpd, R_BODY);
            if(r == NO_HANDLER) {
                r = request_body(client_fd, client_sock);
                if (r == IO_EAGAIN_R || (client_sock->request.method == M_ERROR)) {
                    return IO_EAGAIN_R;
                }
                else if (r == IO_ERROR)
                    return IO_ERROR;
                else if (r == IO_DONE_R) {
                    r = handle_response_with_default_handler(client_sock, httpd);
                    return r;
                }
                else {
                    printf("ERROR: unknown error at R_BODY.\n");
                    exit(1);
                }
            }
            return r;
        }
        default: {
            printf("ERROR: unknown error at default.\n");
            exit(1);
        }
    }
    printf("ERROR: unknown error at handle_request.\n");
    exit(1);
}

//*******************************************  Response模块  ********************************

void send_headers(int client_fd) {
    int buffer_size = 1024;
    char buf[buffer_size];
    memset(buf, 0, buffer_size);
    strcat(buf, "HTTP/1.0 200 OK\r\n");
    strcat(buf, "Server: jmp2httpd 0.1.0\r\n");
    //strcat(buf, "Content-Type: text/html\r\n");
    strcat(buf, "\r\n");
    send(client_fd, buf, strlen(buf), 0);
}

void send_data(int client, char *data) {
    int buffer_size = 1024;
    //不是每次返回一个指针,而是,传给函数一个buff指针,然后对这个指针内容做修改.
    char buf[buffer_size];
    memset(buf, 0, buffer_size);
    strcat(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    strcat(buf, "Server: jmp2httpd 0.1.0\r\n");
    strcat(buf, "Content-Type: text/html\r\n");
    strcat(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    send(client, data, strlen(data), 0);
}

void send_404(int client_fd) {
    int buffer_size = 1024;
    char buf[buffer_size];
    memset(buf, 0, buffer_size);
    strcat(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    strcat(buf, "Server: jmp2httpd 0.1.0\r\n");
    strcat(buf, "Content-Type: text/html\r\n");
    strcat(buf, "\r\n");
    strcat(buf, "<!DOCTYPE html>\r\n<html lang=\"en\">");
    strcat(buf, "<head>\r\n<meta charset=\"UTF-8\">\r\n<title>Not Found</title></head>\r\n");
    strcat(buf, "<body>\r\n<h2 style=\"text-align: center;\"> not found world.</h2>");
    strcat(buf, "<h5 style=\"text-align: right;\">by jmpews.</h5>\r\n");
    strcat(buf, "</body></html>");
    buf[strlen(buf)] = '\0';
    send(client_fd, buf, strlen(buf), 0);
}

int send_file(int client_fd, char *path, long *start) {
    struct stat st;
    FILE *fd;
    int buffer_size = 1024;
    int n, r;
    char buf[buffer_size];
    if ((stat(path, &st) != -1) && ((st.st_mode & S_IFMT) == S_IFREG)) {
        fd = fopen(path, "r");
        if(!fd) {
            printf("ERROR: %s open failed.\n", path);
            exit(1);
        }
        fseek(fd, *start, SEEK_SET);

        while(1) {
            memset(buf, 0, buffer_size);
            n = fread(buf, sizeof(char), buffer_size, fd);
            r = send(client_fd, buf, n, 0);
            if(r == - 1) {
                if (errno == EAGAIN){
                    printf("> EAGAIN: send_file:%s\n", path);
                    fclose(fd);
                    return IO_EAGAIN_W;
                }
                else {
                    printf("ERROR: unknown at send_file.\n");
                    exit(1);
                }
            }
            // 文件读取的字节数和发送的字节数不同
            else if(r != n) {
                printf("ERROR: n!=r at send_file.\n current_cache_length=%ld, r=%d, n=%d", *start, r, n);
                exit(1);
            }
            // 文件读取的字节数和缓冲区的size不一样，需要判断发生了什么？
            else {
                if(n != buffer_size) {
                     if (ferror(fd)) {
                        printf("ERROR: %s reading error.", path);
                    }
                    else if (feof(fd)) {
                        fclose(fd);
                        return IO_DONE_W;
                    }
                }

            }
            *start += n;
       }
    } else {
        printf("ERROR: not found\n");
        exit(1);
    }

}

// 根据状态初始调用handler
int handle_response_with_reqstat(SocketNode *client_sock, ServerInfo *httpd, int reqstat) {
    RouteHandler *rthandler;
    RequestHandler reqhandler;
    int r;
    rthandler = get_route_handler_with_reqstat(httpd->head_route, client_sock->request.request_path, reqstat);
    if(rthandler == NULL) {
        return NO_HANDLER;
        rthandler = (RouteHandler *)httpd->head_route->data;
    }
    
    client_sock->handler = rthandler;
    reqhandler = rthandler->func;
    r = reqhandler(client_sock, httpd);
    
    if(r == IO_DONE_W)
        return IO_DONE_W;
    else if(r == IO_EAGAIN_W) {
        return IO_EAGAIN_W;
    }
    else {
        printf("ERROR: reqhandle error at handle_response_with_handler func.\n");
        exit(1);
    }
}

//对于EAGAIN状态处理的handler
int handle_response_with_handler(SocketNode *client_sock, ServerInfo *httpd) {
    RouteHandler *rthandler;
    RequestHandler reqhandler;
    int r;
    rthandler = client_sock->handler;
    reqhandler = rthandler->func;
    r = reqhandler(client_sock, httpd);
    if(r == IO_DONE_W)
        return IO_DONE_W;
    else if(r == IO_EAGAIN_W) {
        return IO_EAGAIN_W;
    }
    else {
        printf("ERROR: reqhandle error at handle_response_with_handler func.\n");
        exit(1);
    }
}

//默认handler处理
int handle_response_with_default_handler(SocketNode *client_sock, ServerInfo *httpd) {
    RouteHandler *rthandler;
    RequestHandler reqhandler;
    int r;
    rthandler = (RouteHandler *)httpd->head_route->data;
    client_sock->handler = rthandler;
    reqhandler = rthandler->func;
    r = reqhandler(client_sock, httpd);
    
    if(r == IO_DONE_W)
        return IO_DONE_W;
    else if(r == IO_EAGAIN_W) {
        return IO_EAGAIN_W;
    }
    else {
        printf("ERROR: reqhandle error at handle_response_with_handler func.\n");
        exit(1);
    }
}


