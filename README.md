# epoll_server
a high performance chatting server using epoll and thread pool.

## Install
### client编译:
> (1)cd client/
> (2)g++ -I ../include -std=c++11 client.cpp -o client -lpthread

### server编译：
> (1)cd server/
> (2)g++ -I ../include -std=c++11 server.cpp task.cpp thread_pool.cpp main.cpp common.cpp -o server -lpthread

先把初始版本代码push上来，后期填坑。欢迎有兴趣的朋友提供意见或者参与改进。
