#include <iostream>
#include <memory>

#include "server.h"

// main入口函数
int main(int argc, char *argv[]){
    std::unique_ptr<Server> server = Server::create_server();
    return 0;
}