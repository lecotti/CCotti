#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include "tools.h"
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

class Socket
{
private:
    int sockfd;
    char my_ip [INET6_ADDRSTRLEN], peer_ip [INET6_ADDRSTRLEN];
    int my_port, peer_port;

    char* get_my_ip(int sockfd, char* ip);
    int get_my_port(int sockfd);


public:
    Socket(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM, bool server=false);
    Socket(int sockfd, struct sockaddr* addr);
    Socket(const Socket& socket);
    Socket();
    void init (int sockfd, struct sockaddr* addr);

    
    int write(void* msg, int len, int flags=0);
    int read(void* msg, int len, int flags=0);

    int get_sockfd(void) const;

    char* get_peer_ip(char* ip) const;
    char* get_peer_ip(void);
    int get_peer_port(void) const;

    char* get_my_ip(char* ip) const;
    char* get_my_ip(void);
    int get_my_port(void) const;
    
    static char* get_ip(char* ip, struct sockaddr* sa);
    static int get_port(struct sockaddr* sa);

    int operator<< (const char* a);
    int operator<< (int a);
    int operator<< (char a);
    int operator>> (int &a);
    int operator>> (char &a);

    void free(void);
    void close(void);
    ~Socket();

};

#endif // SOCKET_H



