#include "socket.h"


/******************************************************************************
 *  @brief:     Inicializa un signal_handler para cierta "signal".
 * 
 *  @arg:       ip: La dirección IP, en IPv4, IPv6 o DNS. Si es NULL, utilizará
 *              la ip propia del dispositivo (pensado para ser server).
 * 
 *              port: Puerto o protocolo definido en "/etc/services" (por ejemplo,
 *              "http" es el protocolo asignado al puerto 80/tcp).
 * 
 *              family: El tipo de conexión de la IP. Toma tres valores:
 *                  AF_INET:    IPv4
 *                  AF_INET6:   IPv6 
 *                  AF_UNSPEC:  Cualquiera.
 * 
 *              socktype: determina el protocolo, dos tipos:
 *                  SOCK_STREAM:    TCP.
 *                  SOCK_DGRAM:     UDP.
 * 
 *  @return:    None.
 * ***************************************************************************/
Socket::Socket(const char* ip, const char* port, int family, int socktype)
{
    struct addrinfo hints;
    struct addrinfo* res, p;
    int yes=1;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;                       // Tipo de IP.
    hints.ai_socktype = socktype                    // TCP o UDP.
    hints.ai_protocol = 0;                          // El protocolo coincidente con socktype.
    hints.ai_flags = (ip == NULL) ? AI_PASSIVE : 0; // if AI_PASSIVE, escuchar en cualquier IP, pensado para servers.

    if ( getaddrinfo(ip, port, &hints, &res) != 0)
    {
        perror (RED("Getaddrinfo.\n"));
        return;
    }

    for (p = res; p != NULL; p = res->next)
    {
        if ( (this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) ) == -1)
        {
            perror(RED("Socket.\n"));
            continue;
        }

        if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1)
        {
            perrorr(RED("Setsockopt.\n"));
            close(this->sockfd);
            continue;
        }

        if (bind(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror(RED("Bind.\n"));
            close(this->sockfd);
            continue;
        }
    }

    if (res == NULL)
    {
        printf("Could not find any viable socket to connect to.\n");
    }


    
    /*  USER
    if (connect(this->sockfd, this->res->ai_addr, this->res->ai_addrlen) == -1)
    {
        perror(RED("connect.\n"));
        return;
    }

    */

    if (listen(this->sockfd, backlog) == -1)
    {
        perror(RED("Listen.\n");
        return;
    }

    struct sockaddr_storage client_addr;
    socklen_t addrlen = sizeof (struct sockaddr_storage);
    int new_sockfd;

    if ( (new_sockfd = accept(this->sockfd, &client_addr, &sockaddr_storage) ) == -1)
    {
        perror(RED("Accept.\n"));
        return;
    }

}


Socket::~Socket()
{
    freeaddrinfo(this->res);
    close(this->sockfd);
}