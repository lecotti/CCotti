#include "server.h"

Server::Server(char* ip, char* port, void(*client_handler)(Client&), int family, int socktype, int backlog)
{
    this->ignore_childs();
    
    this->socket_init(ip, port, family, socktype);

    if (listen(this->sockfd, backlog) == -1)
    {
        perror(RED("Listen.\n"));
        return;
    }

    while(1)
    {
        this->exe_loop();
    }
}

void Server::ignore_childs (void)
{
    //Signal sig_child(SIGCHLD, SIG_IGN, 0);
}

void Server::socket_init(char* ip, char* port, int family, int socktype)
{
    struct addrinfo hints;
    struct addrinfo* res, *p;
    int yes=1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;           // Tipo de IP.
    hints.ai_socktype = socktype;       // TCP o UDP.
    hints.ai_protocol = 0;              // El protocolo coincidente con socktype.
    hints.ai_flags = (ip == NULL) ? AI_PASSIVE : 0;        // if AI_PASSIVE, escuchar en cualquier IP, pensado para servers.

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

        if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1)
        {
            perror(RED("Setsockopt.\n"));
            close(this->sockfd);
            continue;
        }

        if (bind(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror(RED("Bind.\n"));
            close(this->sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        printf("Could not find any viable socket to connect to.\n");
        return;
    }

    freeaddrinfo(res);
}

void Server::exe_loop(void)
{
    struct sockaddr_storage client_addr;
    socklen_t addrlen = sizeof (struct sockaddr_storage);
    int new_sockfd;

    if ( (new_sockfd = accept(this->sockfd,(struct sockaddr*) &client_addr, &addrlen) ) == -1)
    {
        perror(RED("Accept.\n"));
        return;
    }

    if (!fork())
    {
        Client client(new_sockfd);
        //client_handler(client); TODO agregar como miembro de la clase
        exit(0);
    }
    
    close(new_sockfd);
}

Server::~Server()
{
    close(this->sockfd);
}




    