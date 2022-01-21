#include "server.h"

/******************************************************************************
 *  @brief:     Crea un server, inicializando el socket por el que escuchará las
 *              conexiones.
 * 
 *  @arg:       Same as Socket::Socket().
 * 
 *  @return:    Throws std::runtime_error si hay algún error.
 * ***************************************************************************/
Server::Server(const char* ip, const char* port, int family, int socktype): socket(ip, port, family, socktype, true) {}

/******************************************************************************
 *  @brief:     Inicializa el servidor. Cada vez que se reciba una nueva conexión,
 *              se llamará a "on_accept()", y para seguir escuchando nuevos clientes
 *              "on_quit()" debe retornar false.
 * 
 *  @arg:       backlog: cantidad de conexiones máximas en "lista de espera".
 * 
 *  @return:    Void.
 * ***************************************************************************/
void Server::start(int backlog)
{
    int client_sockfd;
    Socket client_socket;
    struct sockaddr_storage client_addr;
    socklen_t addrlen = sizeof (struct sockaddr_storage);

    Signal::ignore(SIGCHLD);    //Ignoring childs is necessary as i won't be waiting for them.

    if (listen(this->socket.get_sockfd(), backlog) != 0)   //Starting the server.
    {
        perror(ERROR("Couldn't start the server with listen.\n"));
        return;
    }

    while(!this->on_quit())
    {
        if ( (client_sockfd = accept(this->socket.get_sockfd(), (struct sockaddr*) &client_addr, &addrlen) ) == -1)
        {
            perror(WARNING("Couldn't accept connection.\n"));
            continue;
        }

        client_socket.init(client_sockfd, (struct sockaddr *) &client_addr);

        this->on_accept(client_socket);
        
        client_socket.free();
    }

    this->socket.free();
}

/******************************************************************************
 *  @brief:     Función por defecto. El servidor seguirá corriendo hasta que esta
 *              función retorne "true".
 * ***************************************************************************/
bool Server::on_quit(void)
{
    return false;
}

/******************************************************************************
 *  @brief:     Retorna el socket.
 * ***************************************************************************/
Socket& Server::get_socket(void)
{
    return this->socket;
}




    