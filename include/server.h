#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "tools.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include "client.h"
#include "sig.h"


class Server
{
private:
    int sockfd;
public:
    Server(char* ip, char* port, void(*client_handler)(Client&), int family=AF_UNSPEC, int socktype=SOCK_STREAM, int backlog=20);

    void socket_init(char* ip, char* port, int family, int socktype);
    void ignore_childs(void);
    void exe_loop(void);
    ~Server();

};

#endif //SERVER_H