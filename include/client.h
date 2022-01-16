#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "tools.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

class Client
{
private:
    int sockfd;
    struct sockaddr* sa;

public:

    Client(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM);
    Client(int sockfd);
    Client (int sockfd, struct sockaddr* &sa);
    void send(void* msg, int len);
    void recv(void* msg, int len);
    char* get_ip(char* ip);
    int get_port();

    static char* get_ip(char* ip, struct sockaddr* sa);
    static int get_port(struct sockaddr* sa);
    ~Client();

};

#endif //CLIENT_H 