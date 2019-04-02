#ifndef CLIENT_H_
#define CLIENT_H_

int clientfd;
struct sockaddr_in server_addr;
const char *server_ip = "127.0.0.1";

enum{SERVER_PORT = 7000};

/* 函数声明 */
void connect_to_server();
void epoll_loop();

bool recv_msg_from_server(int clientfd, char *buffer, int length, int epollfd);
void recv_msg_from_stdin(bool *running);

#endif