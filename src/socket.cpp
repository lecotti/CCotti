#include "socket.h"

/******************************************************************************
 *  @brief:     Crea un socket.
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
 *              server: Si el socket se va a usar para 
 * 
 *  @return:    Throws std::runtime_error si hubo algún error.
 * ***************************************************************************/
Socket::Socket(const char* ip, const char* port, int family, int socktype, bool server)
{
    struct addrinfo hints;
    struct addrinfo* res, *p;
    int yes=1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;                               // Tipo de IP.
    hints.ai_socktype = socktype;                           // TCP o UDP.
    hints.ai_protocol = 0;                                  // El protocolo coincidente con socktype.
    hints.ai_flags = (ip == NULL) ? AI_PASSIVE : 0;        // if AI_PASSIVE, escuchar en cualquier IP, pensado para servers.

    if ( getaddrinfo(ip, port, &hints, &res) != 0)
    {
        perror (ERROR("Getaddrinfo.\n"));
        throw(std::runtime_error("Getaddrinfo."));
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        if ( (this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) ) == -1)
        {
            perror(WARNING("Failed connection to one of the sockets returned by getaddrinfo with socket.\n"));
            continue;
        }

        if (server)
        {
            if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1)
            {
                perror(WARNING("Critical error while trying to reuse port with setsockopt.\n"));
                ::close(this->sockfd);
                continue;
            }

            if (bind(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                perror(WARNING("Couldn't bind to port with this info.\n"));
                ::close(this->sockfd);
                continue;
            }

            //Como es el server, no hay una "peer ip", así que las inicializo iguales.
            this->get_ip(this->my_ip, p->ai_addr);
            this->my_port   =   this->get_port(p->ai_addr);

            this->get_my_ip(this->peer_ip);
            this->peer_port =   this->get_my_port();
            
        }

        else
        {
            if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                perror(WARNING("Couldn't connect to one of the sockets.\n"));
                ::close(this->sockfd);
                continue;
            }

            this->get_my_ip(this->sockfd, this->my_ip);
            this->my_port   =   this->get_my_port(this->sockfd);

            this->get_ip(this->peer_ip, p->ai_addr);
            this->peer_port =   this->get_port(p->ai_addr);
        }
        
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, ERROR("Couldn't create the socket.\n"));
        throw(std::runtime_error("Socket"));
    }

    freeaddrinfo(res);
}

/******************************************************************************
 *  @brief:     Constructor de copia, usado cuando se llama a "on_accept()" del server.
 * ***************************************************************************/
Socket::Socket(const Socket &socket)
{
    this->sockfd    =   socket.get_sockfd();

    socket.get_my_ip(this->my_ip);
    this->my_port   =   socket.get_my_port();

    socket.get_peer_ip(this->peer_ip);
    this->peer_port =   socket.get_peer_port();
}


/******************************************************************************
 *  @brief:     Instancia un socket ya creado, generalmente usado con los valores
 *              que devuelve "accept".
 * 
 *  @arg:       Los valores retornados por accept.
 * 
 *  @return:    El socket file descriptor.
 * ***************************************************************************/
Socket::Socket(int sockfd, struct sockaddr* addr)
{
    this->init(sockfd, addr);
}

Socket::Socket() {}

void Socket::init (int sockfd, struct sockaddr* addr)
{
    this->sockfd    =   sockfd;

    this->get_my_ip(sockfd, this->my_ip);
    this->my_port   =   this->get_my_port(sockfd);

    this->get_ip(this->peer_ip, addr);
    this->peer_port =   this->get_port(addr);
}

/******************************************************************************
 *  @brief:     Escribe en el socket. Debe estar debidamente conectado.
 * 
 *  @arg:       msg: el mensaje a enviar.
 * 
 *              len: el tamaño del buffer ("msg") en bytes, se escribirá exac-
 *              tamente esta cantidad, el buffer puede ser más grande, pero evitar
 *              que sea más chico.
 * 
 *              flags=0: los flags a la hora de escribir (ver man send).
 * 
 *  @return:    La cantidad de bytes evniados, o "-1" en error. Si el socket
 *              del peer se cerró, levanta la señal SIGPIPE.
 * ***************************************************************************/
int Socket::write(void* msg, int len, int flags)
{
    int bytes_sent = 0;
    int aux;
    do
    {
        if ( (aux = send(this->sockfd, msg, len, flags) ) == -1)
        {
            perror(ERROR("Coudln't send data through the socket with send.\n"));
            bytes_sent = aux;
            break;
        }

        bytes_sent += aux;

    }while (bytes_sent < len);

    return bytes_sent;
}

/******************************************************************************
 *  @brief:     Lee el socket. Debe estar debidamente conectado.
 * 
 *  @arg:       msg: el buffer donde se almacenará el mensaje.
 * 
 *              len: el tamaño del buffer ("msg").
 * 
 *              flags=0: los flags (ver man recv)
 * 
 *  @return:    La cantidad de bytes recibidos, "0" si la conexión del otro lado
 *              se cerró de manera correcta, o "-1" en error.
 * ***************************************************************************/
int Socket::read(void* msg, int len, int flags)
{
    int bytes_read = 0;

    bytes_read = recv(this->sockfd, msg, len, flags);

    if ( bytes_read == -1 )
    {
        perror(ERROR("Couldn't receive the bytes from the socket with recv.\n"));
    }

    else if (bytes_read == 0)
    {
        fprintf(stderr, INFO("The other socket was closed gracefully, or a zero length message was sent.\n"));
    }

    return bytes_read;
}



/******************************************************************************
 *  @brief:     High-level getters del ip y puerto.
 * ***************************************************************************/
char* Socket::get_peer_ip(char* ip) const
{
    strcpy(ip, this->peer_ip);
    return ip;
}

char* Socket::get_peer_ip(void)
{
    return this->peer_ip;
}

int Socket::get_peer_port(void) const
{
    return this->peer_port;
}


char* Socket::get_my_ip(char* ip) const
{
    strcpy(ip, this->my_ip);
    return ip;
}

char* Socket::get_my_ip(void)
{
    return this->my_ip;
}

int Socket::get_my_port() const
{
    return this->my_port;
}

/******************************************************************************
 *  @brief: Retorna el socket file descriptor.
 * ***************************************************************************/
int Socket::get_sockfd(void) const
{
    return this->sockfd;
}

/******************************************************************************
 *  @brief: Forma ágil de escribir y leer en el socket.
 * ***************************************************************************/
int Socket::operator<< (int a)
{
    return(this->write((void*)&a, sizeof(int), 0));
}

int Socket::operator<< (const char* a)
{
    return(this->write((void*)a, strlen(a) + 1, 0));
}

int Socket::operator<< (char a)
{
    return(this->write((void*)&a, sizeof(char), 0));
}

int Socket::operator>> (int &a)
{
    return (this->read((void*)&a, sizeof(int), 0));
}

int Socket::operator>> (char &a)
{
    return (this->read((void*)&a, sizeof(char), 0));
}

/******************************************************************************
 *  @brief: Destructores
 * ***************************************************************************/
void Socket::close(void)
{
    shutdown(this->sockfd, SHUT_RDWR);
    ::close(this->sockfd);
}


void Socket::free(void)
{
    this->close();
}

Socket::~Socket()
{
    this->close();
}

/******************************************************************************
 * MÉTODOS PRIVADOS y ESTÁTICOS
 * ****************************************************************************

/******************************************************************************
 *  @brief:     Retorna la ip a partir de la conexión recibida.
 * 
 *  @arg:       ip: lugar donde se almacenará la Ip. El tamaño del vector debe
 *              ser lo suficientemente grande para almacenar una Ipv4 (INET_ADDRSTRLEN)
 *              o una Ipv6 (INET6_ADDRSTRLEN).
 * 
 *              sa: sockaddr, devuelta por accept().
 * 
 *  @return:    El mismo puntero que el argumento "ip".
 * ***************************************************************************/
char* Socket::get_ip(char* ip, struct sockaddr* sa)
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

/******************************************************************************
 *  @brief:     Retorna el puerto por el que se está conectando el socket.
 * 
 *  @arg:       sa: sockaddr, devuelta por accept().
 * 
 *  @return:    El número de puerto.
 * ***************************************************************************/
int Socket::get_port(struct sockaddr* sa)
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

/******************************************************************************
 *  @brief:     Retorna la IP propia, luego de un llamado exitoso a "connect()"
 *              por parte del cliente.
 * 
 *  @arg:       sockfd: el socket correctamente creado y conectado.
 * 
 *              ip: lugar donde se almacenará la Ip. El tamaño del vector debe
 *              ser lo suficientemente grande para almacenar una Ipv4 (INET_ADDRSTRLEN)
 *              o una Ipv6 (INET6_ADDRSTRLEN).
 * 
 *  @return:    El mismo puntero que el argumento "ip", o NULL si hubo error.
 * ***************************************************************************/
char* Socket::get_my_ip(int sockfd, char* ip)
{
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) != 0)
    {
        perror(WARNING("Couldn't get own ip with getsockname.\n"));
        return NULL;
    }

    return this->get_ip(ip, (struct sockaddr*)&addr);
}


/******************************************************************************
 *  @brief:     Retorna el puerto propio, luego de un llamado exitoso a "connect()"
 *              por parte del cliente.
 * 
 *  @arg:       sockfd: el socket correctamente creado y conectado.
 * 
 *  @return:    El número de puerto o "-1" en caso de error.
 * ***************************************************************************/
int Socket::get_my_port(int sockfd)
{
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) != 0)
    {
        perror(WARNING("Couldn't get own ip with getsockname.\n"));
        return -1;
    }

    return this->get_port((struct sockaddr*)&addr);
}
