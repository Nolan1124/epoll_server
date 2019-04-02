#ifndef TASK_H
#define TASK_H

//bool epoll_delete_event(const int epollfd, const int fd);
//bool epoll_add_event(const int epollfd, const int fd);

class Task {
    protected:
        static int task_id;
    public:
        Task(){}
        virtual ~Task(){}
        virtual int run() = 0;
       
        int get_taskid(){ return task_id; }
};

class ClientTask : public Task{
    private:
        int clientfd;
        void do_recv_msg_from_client();

    public:
        ClientTask();
        explicit ClientTask(int fd);

        virtual ~ClientTask();
        virtual int run();
    
        int get_clientfd(){ return clientfd; }
};

class StdinTask : public Task{
    private:
        std::string data_buffer;

        void do_recv_msg_from_stdin();
    public:
        StdinTask();
        explicit StdinTask(std::string buffer);
        virtual ~StdinTask();
        
        virtual int run();
};

#endif