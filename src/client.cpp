#include "client.h"

Client::Client(const char* ip, const char* port, int family, int socktype)
{
    struct addrinfo hints;
    struct addrinfo* res, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;                       // Tipo de IP.
    hints.ai_socktype = socktype;                   // TCP o UDP.
    hints.ai_protocol = 0;                          // El protocolo coincidente con socktype.
    hints.ai_flags = 0; // if AI_PASSIVE, escuchar en cualquier IP, pensado para servers.

    if ( getaddrinfo(ip, port, &hints, &res) != 0)
    {
        perror (RED("Getaddrinfo.\n"));
        return;
    }

    for (p = res; p != NULL; p = res->ai_next)
    {
        if ( (this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) ) == -1)
        {
            perror(RED("Socket.\n"));
            continue;
        }

        if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror(RED("connect.\n"));
            return;
        }

        break;
    }

    this->sa = p->ai_addr;

    if (p == NULL)
    {
        printf("Could not find any viable socket to connect to.\n");
        return;
    }

    freeaddrinfo(res);

}

Client::Client (int sockfd)
{
    socklen_t addrlen = sizeof (struct sockaddr);

    this->sockfd = sockfd;
    getpeername(sockfd, this->sa, &addrlen);    //Sin inicializar this->sa
}

Client::Client (int sockfd, struct sockaddr* &sa)
{
    this->sockfd = sockfd;
    this->sa = sa;
}

void Client::send(void* msg, int len)
{
    if (::send(this->sockfd, msg, len, 0) == -1)
    {
        perror(RED("Send.\n"));
    }
}

void Client::recv(void* msg, int len)
{
    if (::recv(this->sockfd, msg, len, 0) == -1 )
    {
        perror(RED("recv.\n"));
    }
}

char* Client::get_ip(char* ip, struct sockaddr* sa)
{

    if (sa->sa_family == AF_INET)
    {
        inet_ntop(sa->sa_family, &( ( (struct sockaddr_in*) sa)->sin_addr), ip, INET_ADDRSTRLEN); 
    }
    
    else
    {
        inet_ntop(sa->sa_family, &( ( (struct sockaddr_in6*)sa)->sin6_addr), ip, INET6_ADDRSTRLEN);
    }

    return ip;
}

int Client::get_port(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET)
    {
        return (int) ntohs( ( (struct sockaddr_in*)sa)->sin_port);
    }

    else
    {
        return (int) ntohs( ( (struct sockaddr_in6*)sa)->sin6_port);
    }
}

char* Client::get_ip(char* ip)
{
    return Client::get_ip(ip, this->sa);
}

int Client::get_port()
{
    return Client::get_port(this->sa);
}

Client::~Client()
{
    close(this->sockfd);
}