# minihttpd
快速提供web服务(pure c)，采用epoll。

可以将自己的脚本放到该目录下，每次push，需要时直接clone或pull，就立即可以提供web服务。

# Use it ?

### start httpd
```
gcc -o httpd main.c
./httpd
```

# Update Log
## 2015.11.14
用c语言的完成的web server,采用select实现非阻塞,正在实现epoll.

可以根据get的路径返回html或者not found.

暂时没有处理get参数和get请求.

## 2015.11.16
支持epoll

## 2015.11.21
### fix了一个`send_buffer`的bug:
默认发送缓冲区的buffer是8k字节，send函数仅仅将数据copy到发送缓冲区，如果采用while或者for会导致，发送缓冲区快速充满，因为采用非阻塞的socket会导致直接返回返回EAGAIN(-1)，处理方法就是记录ftell()或者sum(每次read志),重新丢入epoll，等待下次可写，然后根据上次读的位置定位文件指针，继续read.

### 添加模块jmp2string
提供比较全面的编码解码。(目前在`encode img =>base64`有问题)

## 2015.11.23
### 修复了EPOLL的ET模式问题。

ET模式，在于减少事件的频繁响应，只有当socket的状态change的时候，才会发出响应，所以listen_fd接受到accept请求，并不知道具体有几个并发socket请求，所以需要while(accept())直到EAGAIN的errno(读取也是类似).

PS：手动很难模拟并发，一个很类似并发的情况就是在一个html文件下面，引入多个本地js，这些js在加载的时候是并发去请求的。

##2015.12.04
### 规格化返回状态
### 按照非阻塞的方式处理request请求
记录header的处理状态，遇到EAGAIN保存状态，对于包含大量数据的请求头做了预处理。

### 宏定义的合理使用
宏这真是个好东西。

用宏打印log`#define LOG(t) {printf("log[%d]..........\n",t);fflush(stdout);}`

用宏处理`#define FREEBUF(buf) if(buf){free(buf);buf=NULL;}`

## 2015.12.09

添加了一个简单的url路由模块

做read_line，尽量不要在函数引用过多第三方元素.

同样send_data，尽量不要在这种函数中使用太多第三方的变量.

## 2015.12.13
重构，更加规范，并且引入列表。

```
switch(IO_HEADER_STATUS)
    0. IO_HEADER_INIT
    1. IO_HEADER_START
    request_header_start(请求方法，请求路径)
        > read_line()
            如果当前状态，已经是START，需要合并header_line_buffer数据
        > * 读完，调用handle_header_start处理。
        > * 没有读完(最后一位!='\n'&&EAGAIN)，保存读取数据到header_line_buffer，状态为IO_HEADER_START

    2. IO_HEADER_BODY
    request_header_body(各种键值对)
        > read_line()
            如果当前状态，已经是HEADER_BODY，需要合并header_line_buffer数据
        > 匹配队列内关键字
        > 如果read_line返回'\n',处理完毕，设置状态IO_HEADER_BODY

    3. IO_BODY
        handle_body()
        读取request_body
    break;

1. 响应状态处理。
处理函数保存到队列里面，每次处理扫描队列，如果关键字在队列中，调用响应的函数


0. R_RESPONSE
    分析route调用合适函数处理
        * 直接返回
        * 分析request_path.
1. RESPONSE
    恢复response_cache_len
    读取request_path 返回
```

## 2015.12.18
fix bug: 

select的timeout每次会进行重置，所以需要在每次select之前进行重新设置

#### Index
```
Index:http://xxx.xx/index.html
```

#### jsonp 一些xss脚本(from http://root.cool)
```
json:http://xxx.xx/jsonp.js
```

#### jmp2string 提供全面的encode/decode(modify http://faststring.lgvirtual.com/)
```
jmp2string http://xxx.xx/scripts/jmp2string/index.htm
```

#### csrf 收集的关于csrf的知识
```
csrf http://xxx.xx/scripts/csrf/csrf.form.html
```
#### tool.site.txt
```
tools http://xxx.xx/tool.site.txt
`
