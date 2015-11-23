# minihttpd
快速提供web服务(pure c)，采用epoll。

可以将自己的脚本放到该目录下，每次push，需要时直接clone或pull，就立即可以提供web服务。

# Use it ?

### start httpd
```
gcc -o httpd main.c
./httpd
```

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
```

# Day Log
## 2015.11.14
用c语言的完成的web server,采用select实现非阻塞,正在实现epoll.

可以根据get的路径返回html或者not found.

暂时没有处理get参数和get请求.

## 2015.11.16
支持epoll

## 2015.11.21
### fix了一个send_buffer的bug:
默认发送缓冲区的buffer是16384字节，send函数仅仅将数据copy到发送缓冲区，如果采用while或者for会导致，发送缓冲区快速充满，因为采用非阻塞的socket会导致直接返回返回EAGAIN(-1)，处理方法就是记录ftell()或者sum(每次read志),重新丢入epoll，等待下次可写，然后根据上次读的位置定位文件指针，继续read.

### 添加模块jmp2string
提供比较全面的编码解码。(目前在encode img =>base64有问题.)
