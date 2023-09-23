#include "server.h"

bool Server::exit;

/// @brief Creates a server. Uses same parameters as Socket::Socket().
/// @return Might throw std::runtime_error on error.
Server::Server(const char* ip, const char* port, int family, int socktype):
    socket(ip, port, family, socktype, true) {
    Server::exit = false;
    Signal::ignore(SIGCHLD);  // Ignoring childs is necessary to avoid zombies.
    Signal::set_handler(SIGINT, &Server::leave);
}

/// @brief Starts the server, blocks operation. Every time a new connection is
///  received, the function "on_accept()" will be called. The server will keep
///  running until "on_quit()" return true.
/// @param backlog Number of clients that can be put "on hold".
void Server::start(int backlog) {
    int client_sockfd;
    Socket client_socket;
    struct sockaddr_storage client_addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    int buff;

    this->backlog = backlog;
    while(!Server::exit) {
        this->on_start();
        if (listen(this->socket.get_sockfd(), this->backlog) != 0) {
            perror(ERROR("Couldn't start the server with listen"));
            return;
        }
        if ( (client_sockfd = accept(this->socket.get_sockfd(), (struct sockaddr*) &client_addr, &addrlen) ) == -1) {
            if (errno != EINTR) {
                // The accept was NOT terminated by a signal
                perror(WARNING("Couldn't accept a connection from a client"));
            }
            continue;
        }
        if (client_socket.init(client_sockfd, (struct sockaddr*) &client_addr) == -1) {
            client_socket.close();
            continue;
        }
        this->on_new_client();
        if ((buff = fork()) == -1) {
            perror(ERROR("fork in Server::start. Failed to create child"));
            client_socket.close();
            continue;
        } else if (buff == 0) {
            this->on_accept(client_socket);
            client_socket.close();
            ::exit(0);
        }
    }
    this->socket.close();
    this->on_quit();
}

/// @brief Returns the socket
Socket& Server::get_socket(void) {
    return this->socket;
}

/// @brief Handler for SIGINT signal. Makes the server end.
void Server::leave(int) {
    Server::exit = true;
}
