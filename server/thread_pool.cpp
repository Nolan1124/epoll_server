#include <iostream>
#include <pthread.h>
#include <memory>
#include <vector>
#include <unistd.h>

#include "thread_pool.h"

//私有构造函数实现
ThreadPool::ThreadPool(int num) : threads_num(num){
    is_running = true;
    pthread_mutex_init(&mutex_task, nullptr);
    pthread_cond_init(&cond_task, nullptr);

    create_threads();
    std::cout << "create threads poll succeed." << std::endl;
}

ThreadPool::~ThreadPool(){
    is_running = false;
    pthread_cond_broadcast(&cond_task);
}

std::unique_ptr<ThreadPool> ThreadPool::create_thread_pool(int number){
    std::unique_ptr<ThreadPool> thread_pool_instance(new ThreadPool(number));
    return thread_pool_instance;
}

void ThreadPool::create_threads(){
    try
    {
        pthread_t tid;
        for(int i = 0; i < threads_num; i++){
            pthread_create(&tid, nullptr, thread_func, this);
            threads_set.push_back(tid);
        } 
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void ThreadPool::add_task(Task *task){
    pthread_mutex_lock(&mutex_task);//进入任务队列临界区
    task_queue.push(task);
    pthread_mutex_unlock(&mutex_task);
    pthread_cond_signal(&cond_task);//通知阻塞在cond_task上的线程，即空闲线程取任务
}

Task *ThreadPool::take_task(){
    Task *ptask = nullptr;

    //一直循环，直到取到一个任务为止
    while(ptask == nullptr){
        pthread_mutex_lock(&mutex_task);//进入临界区

        while(task_queue.empty() && is_running){
            pthread_cond_wait(&cond_task, &mutex_task);
        }

        if(!is_running){
            pthread_mutex_unlock(&mutex_task);
            break;
        }else if(task_queue.empty()){
            pthread_mutex_unlock(&mutex_task);
            continue;
        }

        ptask = task_queue.front();
        task_queue.pop();

        std::cout << "thread " << pthread_self() << " get the task " << ptask->get_taskid() << std::endl;
        
        pthread_mutex_unlock(&mutex_task);//退出临界区
    }

    return ptask;
}
    
void *ThreadPool::thread_func(void *arg){
    ThreadPool *pool = (ThreadPool *)arg;

    while(pool -> is_running){
        Task *task = pool -> take_task();

        if(!task){
            break;
        }//线程池关闭，线程自动退出

        task -> run();

        delete task;
    }
}

