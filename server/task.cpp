#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

#include "task.h"
#include "server.h"
#include "common.h"

//静态成员函数初始化
int Task::task_id = 0;

/*ClientTask*/
ClientTask::ClientTask(){
    task_id++;
}

ClientTask::ClientTask(int fd) : clientfd(fd){
    task_id++;
}

ClientTask::~ClientTask(){
    std::cout << "ClientTask destroy" << std::endl; 
    task_id--;
}

int ClientTask::run(){
    this -> do_recv_msg_from_client();
}

void ClientTask::do_recv_msg_from_client(){
    char buffer[1024];
    int data_len = 0;
    
    int clients_num;
    std::vector<int> clients;
    std::string send_buffer = "";

    memset(buffer, 0, sizeof(buffer));
    data_len = recv(clientfd, buffer, sizeof(buffer), 0);

    if(data_len < 0){
        std::cerr << "read client data failed.error clientfd:" << clientfd << std::endl;
    }else if(data_len == 0) {
        if(epoll_delete_event(Server::get_epoll_fd(), clientfd)){
            pthread_mutex_lock(&mutex_client);
            
            clients_num = Server::get_client_num();
            clients_num--;
            Server::set_client_num(clients_num);

            for(std::vector<int>::iterator it = clients.begin(); it != clients.end();){
                if(*it == clientfd){
                    it = clients.erase(it);
                }else{
                    it++;
                }
            }

            close(clientfd);
            pthread_mutex_unlock(&mutex_client);
            
            std::cout << "client " << clientfd << " quit." << std::endl;
        }else {
            std::cerr << "delete clientfd " << clientfd << "failed." << std::endl;
        }
    }else {
        //构造广播数据
        send_buffer.append("From Client ")
                    .append(std::to_string(clientfd))
                    .append(" :")
                    .append(buffer);
        std::cout << "send buffer:" << send_buffer << std::endl;

        pthread_mutex_lock(&mutex_client);
        clients_num = Server::get_client_num();
        clients = Server::get_clients();

        for(int i = 0; i < clients.size(); i++){
            if(clientfd != clients[i]){
                send(clients[i], send_buffer.c_str(), send_buffer.size(), 0);
            }
        }

        pthread_mutex_unlock(&mutex_client);
        send_buffer.erase();
    }

}

/*StdinTask*/
StdinTask::StdinTask(){
    task_id++;
}

StdinTask::StdinTask(std::string buffer) : data_buffer(buffer){
    task_id++;
}

StdinTask::~StdinTask(){
    std::cout << "StdinTask destroy" << std::endl; 
    task_id--;
}

int StdinTask::run(){
    this -> do_recv_msg_from_stdin();
}

void StdinTask::do_recv_msg_from_stdin(){
    std::string buffer = this -> data_buffer;

    if(strncmp(buffer.c_str(), "number", strlen("number")) == 0){
        std::cout << "connected clients number is : " << Server::get_client_num() << std::endl;
    } 
    /*else if(strncmp(stdin_param -> buffer.c_str(), "quit", strlen("quit")) == 0){
        //fix-me:更完善的关闭流程
        *(stdin_param ->is_running) = false;
        std::cout << "server is shutdown." << std::endl;
    }*/
    else{
        std::cout << "invalid command!" << std::endl;
    }
}
