#include <iostream>
#include <sys/socket.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "common.h"

int main(int argc, const char *argv[]){
    connect_to_server();
    epoll_loop();

    return 0;
}

void connect_to_server(){
    int result;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(SERVER_PORT);

    clientfd = socket(AF_INET, SOCK_STREAM, 0);

    if(clientfd == -1){
        std::cerr << "socket failed." << std::endl;
        exit(1);
    }

    result = connect(clientfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));

    if(result == -1){
        std::cerr << "connect to server failed." << std::endl;
        exit(1);
    }
}

void epoll_loop(){
    bool running = true;

    int epollfd = epoll_create(3);

    if(epollfd == -1){
        std::cerr << "epoll_create failed." << std::endl;
        return;
    }

    if(epoll_add_event(epollfd, clientfd) == false){
        std::cerr << "errno:" << errno << ";epoll_add_server_sockfd failed." << std::endl;
        return;
    }
    if(epoll_add_event(epollfd, STDIN_FILENO) == false){
        std::cerr << "errno:" << errno << ";epoll_add_STDIN failed." << std::endl;
        return;
    }

    while(running){
        int event_num;
        char buffer[1024];
        struct epoll_event events[3];

        event_num = epoll_wait(epollfd, events, 3, -1);

        if(event_num < 0){
            std::cerr << "epoll_wait failed." << std::endl;
            break;
        }

        memset(buffer, 0, sizeof(buffer));

        for(int i = 0; i < event_num; i++){
            int fd = events[i].data.fd;
            int current_event = events[i].events;

            if(fd == STDIN_FILENO){
                if(current_event & EPOLLIN){
                    recv_msg_from_stdin(&running);
                    if(!running){
                        break;
                    }
                }

            }else if(fd == clientfd){
                if(current_event & EPOLLIN){
                    if(!recv_msg_from_server(clientfd, buffer, sizeof(buffer),epollfd)){
                        std::cerr << "recv msg from server failed." << std::endl;
                        continue;
                    }

                    std::cout << buffer << std::endl;
                }
            }
        }
    }

    close(clientfd);
    close(epollfd);
}

/*处理不同事件响应：stdin、接受服务器数据、向服务器发送数据*/
bool recv_msg_from_server(int clientfd, char *buffer, int length, int epollfd){
    int data_len = recv(clientfd, buffer, length - 1, 0);

    if(data_len < 0){
        std::cerr << "read server data failed." << std::endl;
        return false;
    }else if(data_len == 0) {
        std::cout << "read data finished!" << std::endl;
    }

    return true;
}

void recv_msg_from_stdin(bool *running){
    std::string buffer;

    std::cin >> buffer;

    if(buffer == "quit"){
        *running = false;
        std::cout << "client is shutdown. " << std::endl;
    }else {
        if(send(clientfd, buffer.c_str(), sizeof(buffer), 0) < 0){
            std::cerr << "send failed." << std::endl;
        }
    }
}