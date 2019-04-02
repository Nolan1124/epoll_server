#include <sys/epoll.h>

bool epoll_add_event(const int epollfd, const int fd){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1)
        return false;

    return true;
}

bool epoll_delete_event(const int epollfd, const int fd){
    struct epoll_event event;

    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;

    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event) == -1)
        return false;

    return true;

}

bool epoll_modify_event(const int epollfd, const int fd){
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;

    if(epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) == -1)
        return false;

    return true;
}