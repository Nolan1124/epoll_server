#ifndef SERVER_H_
#define SERVER_H_
#include <memory>

#include "thread_pool.h"

enum {QUEUE = 512, events_max_size = 1024, SERVER_PORT = 7000};

extern pthread_mutex_t mutex_client;

class Server{
    private:
        bool running;//服务运行标志
        int listen_sockfd;//监听文件描述符
        unsigned int threads_num;

        static int epoll_fd;
        static std::vector<int> clients;//用于存储已连接的客户端描述符
        static int client_num;//已连接客户端数目
        
        std::unique_ptr<ThreadPool> thread_pool;//线程池对象

        /*将构造函数设置为private，实现单例模式*/
        Server();
        explicit Server(unsigned int number);

        void set_running(bool is_running) { running = is_running; }
        void set_listen_sockfd(int fd) { listen_sockfd = fd; }

        bool listen_socket_init();//初始化监听socket
        bool server_epoll_init();//初始化epoll
        void server_main_loop();
        
        bool do_client_connect_event();

        void shutdown_server();
        void shutdown_all_clients();//关闭所有已连接客户端
        
    public:
        virtual ~Server(){}

        void server_init();
        bool set_noblocking_mode(int fd);
        
        static std::unique_ptr<Server> create_server();

        static int get_client_num() { return client_num; }
        static void set_client_num(int number) { client_num = number; }
        //static bool &get_server_status() { return running; }
        //static int &get_listen_sockfd() { return listen_sockfd; }
        static int &get_epoll_fd() { return epoll_fd; }
        static std::vector<int> &get_clients(){ return clients; }

};

#endif