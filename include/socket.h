#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "tools.h"

class Socket
{
private:
    struct addrinfo* res;

public:
    Socket(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM);

};

#endif // SOCKET_H



