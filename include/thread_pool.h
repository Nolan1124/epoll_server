#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <queue>

#include "task.h"

class ThreadPool
{
private:
    unsigned int max_num; //最大线程数
    unsigned int min_num; //最小线程数

    bool is_running = false;
    int threads_num; //线程数目
    std::vector<pthread_t> threads_set;//线程数组
    std::queue<Task *> task_queue;//任务队列
    
    pthread_mutex_t mutex_task; //互斥锁
    pthread_cond_t cond_task; //取任务条件变量

    //单例模式实现,保证线程池全局唯一
    ThreadPool(){}
    explicit ThreadPool(int number = 5);

    void create_threads();
    Task *take_task();//从任务队列中取走任务
    static void *thread_func(void *arg);//线程函数

public:
    ~ThreadPool();

    void add_task(Task *task);//向线程池任务队列中添加任务
    static std::unique_ptr<ThreadPool> create_thread_pool(int number = 10);
    int get_thread_number() {return threads_num;}
    
};

#endif
