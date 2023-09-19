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

class Socket {
private:
    int sockfd;
    char my_ip [INET6_ADDRSTRLEN], peer_ip [INET6_ADDRSTRLEN];
    int my_port, peer_port;
    int get_ip_from_sockfd(int sockfd, char* ip);
    int get_port_from_sockfd(int sockfd);
    int get_ip_from_sockaddr(char* ip, struct sockaddr* sa);
    int get_port_from_sockaddr(struct sockaddr* sa);

public:
    Socket(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM, bool server=false);
    Socket(const Socket& socket);
    Socket();
    int init (int sockfd, struct sockaddr* addr);
    static bool is_listening(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM);
    void close(void);
    ~Socket();

    int write(void* msg, int len, int flags=0);
    int read(void* msg, int len, int flags=0);

    int get_sockfd(void) const;
    void get_peer_ip(char* ip) const;
    int get_peer_port(void) const;
    void get_my_ip(char* ip) const;
    int get_my_port(void) const;

    Socket& operator<< (const char* a);
    Socket& operator<< (int a);
    Socket& operator<< (char a);
    Socket& operator>> (int &a);
    Socket& operator>> (char &a);
};

#endif // SOCKET_H
