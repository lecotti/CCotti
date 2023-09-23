#include "socket.h"

/******************************************************************************
 * Constructors and initialization
******************************************************************************/

/// @brief Creates a socket.
/// @param ip The IP address of the device, as a string. It can be IPv4 or IPv6.
///  If null, it will use any IP from the device.
/// @param port Port number as a string, or any protocol defined in "/etc/services".
///   For example, "http" is the protocol related to the port "80/tcp".
/// @param family It can be:
///  * AF_INET;   For IPv4.
///  * AF_INET6;  For IPv6.
///  * AF_UNSPEC; Any.
/// @param socktype It can be:
///  * SOCK_STREAM; For TCP.
///  * SOCK_DGRAM;  For UDP.
/// @param server If "true", this socket will be opened to be used as a server.
///  If "false", it will be used to connect to other socket.
/// @return Might throw std::runtime_error on error.
Socket::Socket(const char* ip, const char* port, int family, int socktype, bool server) {
    struct addrinfo hints;
    struct addrinfo* res, *p;
    int yes=1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;       // IP type.
    hints.ai_socktype = socktype;   // TCP o UDP.
    hints.ai_protocol = 0;          // If "0", same protocol as "socktype".
    hints.ai_flags = (ip == NULL) ? AI_PASSIVE : 0; // if "AI_PASSIVE", listen on any IP.
    if (getaddrinfo(ip, port, &hints, &res) != 0) {
        perror (ERROR("getaddrinfo in Socket::Socket"));
        throw(std::runtime_error("getaddrinfo"));
    }
    for (p = res; p != NULL; p = p->ai_next) {
        if ( (this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) ) == -1) {
            perror(WARNING("socket in Socket::Socket. Failed connection to one of the sockets"));
            continue;
        }
        if (server) {
            if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes) ) == -1) {
                perror(WARNING("setsockopt in Socket::Socket. Trying to reuse port"));
                ::close(this->sockfd);
                continue;
            }
            if (bind(this->sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                perror(WARNING("bind in Socket::Socket"));
                ::close(this->sockfd);
                continue;
            }
            // For a server socket, own IP and peer IP are equal.
            if (this->get_ip_from_sockaddr(this->my_ip, p->ai_addr) == -1) {
                continue;
            }
            this->my_port = this->get_port_from_sockaddr(p->ai_addr);
            this->get_my_ip(this->peer_ip);
            this->peer_port = this->get_my_port();
        } else {
            if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                perror(WARNING("Couldn't connect to one of the sockets"));
                ::close(this->sockfd);
                continue;
            }
            this->get_ip_from_sockfd(this->sockfd, this->my_ip);
            this->my_port = this->get_port_from_sockfd(this->sockfd);
            this->get_ip_from_sockaddr(this->peer_ip, p->ai_addr);
            this->peer_port = this->get_port_from_sockaddr(p->ai_addr);
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, ERROR("Couldn't create the socket"));
        throw(std::runtime_error("Socket"));
    }
    freeaddrinfo(res);
}

/// @brief Copy constructor.
Socket::Socket(const Socket &socket) {
    this->sockfd = socket.get_sockfd();
    socket.get_my_ip(this->my_ip);
    this->my_port = socket.get_my_port();
    socket.get_peer_ip(this->peer_ip);
    this->peer_port = socket.get_peer_port();
}

/// @brief Empty constructor. Must call Socket::init(). Used after a successful
///  call to "accept".
Socket::Socket() {}

/// @brief Creates a socket from a successful call to "accept()".
/// @param sockfd Socket file descriptor.
/// @param addr struct sockaddr from the "accept()" call.
/// @return "0" on success, "-1" on error.
int Socket::init(int sockfd, struct sockaddr* addr) {
    this->sockfd = sockfd;
    if (this->get_ip_from_sockfd(sockfd, this->my_ip) == -1) {
        return -1;
    }
    this->my_port = this->get_port_from_sockfd(sockfd);
    if (this->get_ip_from_sockaddr(this->peer_ip, addr) == -1) {
        return -1;
    }
    this->peer_port = this->get_port_from_sockaddr(addr);
    return 0;
}

/// @brief Returns "true" if the socket is listening for connections. Same
///  parameters as constructor. The server will get a "recv()" with a "0" return
/// value when this function is called.
/// @return "true" if the socket is listening for connections.
bool Socket::is_listening(const char* ip, const char* port, int family, int socktype) {
    struct addrinfo hints;
    struct addrinfo* res, *p;
    int sockfd;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_protocol = 0;
    hints.ai_flags =  0;
    if (getaddrinfo(ip, port, &hints, &res) != 0) {
        return false;
    }
    for (p = res; p != NULL; p = p->ai_next) {
        if ( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol) ) == -1) {
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            ::close(sockfd);
            continue;
        } else {
            ::close(sockfd);
            freeaddrinfo(res);
            return true;
        }
    }
    ::close(sockfd);
    freeaddrinfo(res);
    return false;
}

/******************************************************************************
 * Destructors and cleanup
******************************************************************************/

/// @brief Closes the socket. Connection is not closed gracefully.
Socket::~Socket() {
    ::close(this->sockfd);
}

/// @brief Closes connection and cleans resources.
void Socket::close(void) {
    if (shutdown(this->sockfd, SHUT_RDWR) == -1) {
        perror(ERROR("shutdown in Socket::close"));
    }
    if (::close(this->sockfd) == -1) {
        perror(ERROR("close in Socket::close"));
    }
}

/******************************************************************************
 * Read and write functions
******************************************************************************/

/// @brief Writes to the socket.
/// @param msg Message to send.
/// @param len Length of the message in bytes.
/// @param flags See "man send" for all possible flags ("0" by default).
/// @return Amount of bytes sent, or "-1" on error. If the socket was closed
///  by the peer, it will raise the signal "SIGPIPE".
int Socket::write(void* msg, int len, int flags) {
    int bytes_sent = 0;
    int aux;
    do {
        // Don't generate SIGPIPE, return with -1 if peer was closed
        if ( (aux = send(this->sockfd, msg, len, flags | MSG_NOSIGNAL) ) == -1) {
            perror(ERROR("send in Socket::write"));
            bytes_sent = aux;
            break;
        }
        bytes_sent += aux;
    }while (bytes_sent < len);
    return bytes_sent;
}

/// @brief Reads from the socket.
/// @param msg Buffer where the message will be stored.
/// @param len Length of the buffer "msg".
/// @param flags See "man recv" ("0" by default).
/// @return The amount of bytes received. "0" if the connection was closed
///  correctly from the other end, or "-1" on error.
int Socket::read(void* msg, int len, int flags) {
    int bytes_read = 0;
    bytes_read = recv(this->sockfd, msg, len, flags);
    if ( bytes_read == -1 ) {
        perror(ERROR("recv in Socket::read"));
    } // else if (bytes_read == 0) {
    //     fprintf(stderr, INFO("The other socket was closed gracefully, or a zero length message was sent.\n"));
    // }
    return bytes_read;
}

/******************************************************************************
 *  Setters and getters
******************************************************************************/

/// @brief Copy the IP of the connected peer.
/// @param ip Where the IP will be copied.
void Socket::get_peer_ip(char* ip) const {
    strcpy(ip, this->peer_ip);
}

/// @brief Return peer's port.
int Socket::get_peer_port(void) const {
    return this->peer_port;
}

/// @brief Copy your own IP.
/// @param ip Where the IP will be copied.
void Socket::get_my_ip(char* ip) const {
    strcpy(ip, this->my_ip);
}

/// @brief Return your own port.
int Socket::get_my_port() const {
    return this->my_port;
}

/// @brief Return the socket file descriptor.
int Socket::get_sockfd(void) const {
    return this->sockfd;
}

/******************************************************************************
 *  Overloaded operators
******************************************************************************/

Socket& Socket::operator<< (int a) {
    if (this->write((void*)&a, sizeof(int), 0) == -1) {
        throw(std::runtime_error(""));
    }
    return *this;
}

Socket& Socket::operator<< (const char* a) {
    if (this->write((void*)a, strlen(a) + 1, 0) == -1) {
        throw(std::runtime_error(""));
    }
    return *this;
}

Socket& Socket::operator<< (char a) {
    if (this->write((void*)&a, sizeof(char), 0) == -1) {
        throw(std::runtime_error(""));
    }
    return *this;
}

Socket& Socket::operator>> (int &a) {
    if (this->read((void*)&a, sizeof(int), 0) <= 0) {
        throw(std::runtime_error(""));
    }
    return *this;
}

Socket& Socket::operator>> (char &a) {
    if (this->read((void*)&a, sizeof(char), 0) <= 0) {
        throw(std::runtime_error(""));
    }
    return *this;
}

/******************************************************************************
 * Private methods
******************************************************************************/

/// @brief Gets the Ip value from a sockaddr. Used after an "accept()" call to
///  get the peer's IP, or after a "getaddrinfo() to know your own."
/// @param ip Place where the IP will be stored. It's size must be large enough
///  to store an IPv4 (INET_ADDRSTRLEN) or and Ipv6 (INET6_ADDRSTRLEN).
/// @param sa struct sockaddr, returned by "accept()" or "getaddrinfo()".
/// @return "0" on success, "-1" on error.
int Socket::get_ip_from_sockaddr(char* ip, struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        if (inet_ntop(sa->sa_family, &(((struct sockaddr_in*) sa)->sin_addr), ip, INET_ADDRSTRLEN) == NULL) {
            perror(ERROR("inet_ntop in Socket::get_ip_from_sockaddr"));
            return -1;
        }
    } else if (inet_ntop(sa->sa_family, &(((struct sockaddr_in6*)sa)->sin6_addr), ip, INET6_ADDRSTRLEN) == NULL) {
            perror(ERROR("inet_ntop in Socket::get_ip_from_sockaddr"));
            return -1;
    }
    return 0;
}

/// @brief Returns the port number. Used after an "accept()" cal to get the
///  peer's port, or after a "getaddrinfo()" to know your own.
/// @param sa struct sockaddr, as gotten from "accept()" or "getaddrinfo()".
/// @return Port number. Always succeeds.
int Socket::get_port_from_sockaddr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return (int) ntohs(((struct sockaddr_in*)sa)->sin_port);
    } else {
        return (int) ntohs(((struct sockaddr_in6*)sa)->sin6_port);
    }
}

/// @brief Get your own IP address after a successful call to "connect()".
/// @param sockfd Socket file descriptor.
/// @param ip Where the IP will be stored. Its size should be "INET_ADDRSTRLEN"
///  for Ipv4, or "INET6_ADDRSTRLEN" for Ipv6.
/// @return "0" on success, "-1" on error.
int Socket::get_ip_from_sockfd(int sockfd, char* ip) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) != 0) {
        perror(ERROR("getsockname in Socket::get_ip_from_sockfd"));
        return -1;
    }
    this->get_ip_from_sockaddr(ip, (struct sockaddr*)&addr);
    return 0;
}

/// @brief Get your own port after a successful call to "connect()".
/// @param sockfd Socket file descriptor.
/// @return The port number, or "-1" on error.
int Socket::get_port_from_sockfd(int sockfd) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) != 0) {
        perror(ERROR("getsockname in Socket::get_port_from_sockfd"));
        return -1;
    }
    return this->get_port_from_sockaddr((struct sockaddr*)&addr);
}
