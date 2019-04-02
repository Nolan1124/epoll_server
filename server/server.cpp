#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <vector>
#include <fcntl.h>

#include "server.h"
#include "common.h"

//全局变量定义
pthread_mutex_t mutex_client;
int taskid = 0;

//静态成员初始化
int Server::epoll_fd = 0;
std::vector<int> Server::clients = {0};
int Server::client_num = 0;

/*构造函数：默认线程池开启5个线程*/
Server::Server() : running(true){
    pthread_mutex_init(&mutex_client, nullptr);
    thread_pool = ThreadPool::create_thread_pool();

    listen_socket_init();
    server_epoll_init();
    server_main_loop();
}

Server::Server(unsigned int number) : threads_num(number), running(true){
    pthread_mutex_init(&mutex_client, nullptr);
    thread_pool = ThreadPool::create_thread_pool(threads_num);

    listen_socket_init();
    server_epoll_init();
    server_main_loop();
}

/* 初始化服务器端监听socket */
bool Server::listen_socket_init(){
    struct sockaddr_in server_sockaddr;
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(listen_sockfd == -1){
        std::cerr << "socket failed." << std::endl;
        return false;
    }

    memset(&server_sockaddr, 0, sizeof(server_sockaddr));
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(SERVER_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_sockfd, (sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1){
        std::cerr << "bind socket failed!" << std::endl;
        return false;
    }

    if(listen(listen_sockfd, QUEUE) == -1){
        std::cerr << "listen socket failed!" << std::endl;

        return false;
    }

    return true;
}

/*epoll初始化*/
bool Server::server_epoll_init(){
    epoll_fd = epoll_create(events_max_size);

    if(epoll_fd == -1){
        std::cerr << "epoll_create failed." << std::endl;
        return false;
    }

    //添加监听事件
    if(epoll_add_event(epoll_fd, listen_sockfd) == false){
        std::cerr << "errno:" << errno << ";epoll_add_listen_sockfd failed." << std::endl;
        return false;
    }

    //添加标准输入事件
    if(epoll_add_event(epoll_fd, STDIN_FILENO) == false){
        std::cerr << "errno:" << errno << ";epoll_add_STDIN failed." << std::endl;
        return false;
    }

    //设置非阻塞模式
    set_noblocking_mode(listen_sockfd);
    set_noblocking_mode(STDIN_FILENO);

    return true;
}

/*服务器主循环：epoll实现*/
void Server::server_main_loop(){
    int event_num;
    struct epoll_event events[events_max_size];

    if(!server_epoll_init()){
        return; 
    }

    while(running){
        event_num = epoll_wait(epoll_fd, events, events_max_size, -1);
        if(event_num < 0){
            std::cerr << "epoll_wait failed." << std::endl;
            break;
        }

        for(int i = 0; i < event_num; i++){
            int fd = events[i].data.fd;
            int current_event = events[i].events;

            if(fd == STDIN_FILENO){
                if(current_event & EPOLLIN){//从标准输入读 事件
                    std::cout << "get a STDIN event." << std::endl;

                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));

                    int len = read(STDIN_FILENO, buffer, sizeof(buffer));
                    buffer[len] = '\0';

                    Task *stdin_task = new StdinTask(buffer);

                    if(thread_pool != nullptr){
                        thread_pool -> add_task(stdin_task);
                    }
                    
                    if(!running){
                        break;
                    }
                }

            }else if(fd == listen_sockfd){
                if(current_event & EPOLLIN){
                    std::cout << "get a listen socket event." << std::endl;

                    if(!do_client_connect_event()){
                        std::cout << "client connect failed." << std::endl;
                    }
                }

            }else {
                if(current_event & EPOLLIN){
                    std::cout << "get a client socket event." << std::endl;

                    Task *client_task = new ClientTask(fd);
                    
                    if(thread_pool != nullptr){
                        thread_pool -> add_task(client_task);
                    }
                }
            }

        }
    }

    shutdown_server();
}

//单例模式创建服务实例
std::unique_ptr<Server> Server::create_server(){
    std::unique_ptr<Server> server_instance(new Server());
    return server_instance;
}

void Server::shutdown_server(){
    running = false;

    shutdown(listen_sockfd, SHUT_RD);
    close(epoll_fd);
    shutdown_all_clients();   
}

/*监听并处理新客户端连接*/
bool Server::do_client_connect_event(){
    int clientfd;
    struct sockaddr_in client_sockaddr;
    socklen_t length = sizeof(client_sockaddr);

    if((clientfd = accept(listen_sockfd, (struct sockaddr *)&client_sockaddr, &length)) == -1){
        std::cerr << "accept failed!" << std::endl;
        return false;
    }

    if(set_noblocking_mode(clientfd) == false){
        std::cout << "set noblocking mode failed." << std::endl;
        return false;
    }

    if(epoll_add_event(epoll_fd, clientfd) == false){
        close(clientfd);
        return false;
    }
    
    pthread_mutex_lock(&mutex_client);
    client_num++;
    clients.push_back(clientfd);
    pthread_mutex_unlock(&mutex_client);

    std::cout << "client " << clientfd << " connected." << std::endl;

    return true;
}

bool Server::set_noblocking_mode(int fd){
    int flag;

    if((flag = fcntl(fd, F_GETFL, 0)) < 0)
        return false;

    flag |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, 0) < 0){
        return false;
    }

    return true;
}

void Server::shutdown_all_clients(){
    for(int clientfd : clients){
        if(clientfd >= 0){
            close(clientfd);
        }
    }
}
