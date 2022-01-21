#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "socket.h"
#include "sig.h"
#include "tools.h"

/******************************************************************************
 *  Abstract class. Se espera que el usuario herede de esta clase y:
 * 
 *      El constructor: llamando en la lista inicializadora al constructor de Server().
 *      El usuario puede crear tantas atributos de clase o métodos como quiera.
 * 
 *      on_accept(): Función que es llamada cada vez que se recibe la conexión de un
 *      nuevo cliente.
 * 
 *      on_quit(): Opcional. Esta función es llamada luego de que se reciba una nueva
 *      conexión y sea tratada por "on_accept()". El servidor seguirá funcionando 
 *      hasta que esta función retorne "true". Por defecto, "on_quit()" retorna
 *      siempre "false".
 *      
 *      El destructor (si lo necesita).
 *      
 * ***************************************************************************/
class Server
{
private:
    Socket socket;

protected:
    virtual void on_accept(Socket socket) = 0; 

    virtual bool on_quit(void);

public:
    Server(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM);

    void start(int backlog=20);

    Socket& get_socket(void);
};

#endif //SERVER_H

/******************************************************************************
 *  Example usage:
 * ***************************************************************************/
/*
class ExampleServer : public Server
{
protected:
    int value;

    void on_accept(Socket socket) override
    {
        int msg;
        
        socket >> msg;

        this->value += msg;
    }

    bool on_quit(void) override
    {
        return (this->value == 10);
    }

public:
    CreationServer(const char* ip, const char* port):Server(ip, port)
    {
        this->value = 0;
    }
};
*/