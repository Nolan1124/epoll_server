#ifndef COMMON_H_
#define COMMON_H_

bool epoll_add_event(const int epollfd, const int fd);
bool epoll_delete_event(const int epollfd, const int fd);
bool epoll_modify_event(const int epollfd, const int fd);

#endif