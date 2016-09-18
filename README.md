## minihttpd

采用Pure C 写的基于事件循环的HTTP Server，具有EAGAIN处理、简单的路由等，麻雀虽小五脏俱全。

## Features

1. 事件循环机制处理请求
2. 状态机模式处理请求
3. 可以进行基本的路由处理
3. 提供类似[transfer.sh](http://transfer.sh) 的`curl --upload`的文件上传

## Use it ?

### start httpd
```
gcc main.c handlers.c utils.c typedata.c -o httpd
./httpd --header --body --tips
```

### example

```
# 请求基本静态文件
curl localhost:8000

# 上传文件
curl --upload-file ./test http://127.0.0.1:8000/upload/test

# 下载文件
wget http://127.0.0.1:8000/download/jmp.2016.9.18.8.27.37.test
```
