#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "socket.h"
#include "sig.h"
#include "tools.h"

/// @brief Abstract class. The user should inherit from this class and can:
///  * Modify the constructor, as long as the parent constructor is called in the
///  initializer list.
///  * Define the on_accept() function to handle client connections.
///  * Override the on_start() function to make something right before accepting connections.
///  * Override the on_quit() function to make some cleanups after the server exits.
///  The server stops execution after receiving a SIGINT.
class Server {
private:
    Socket socket;

protected:
    // Define this function to handle clients' connections.
    virtual void on_accept(Socket& socket) = 0;
    // Override this function to make something right before accepting connections,
    // but the server is already up.
    virtual void on_start(void) {};
    // Override this function to make some cleanups after the server exits.
    virtual void on_quit(void) {};
    static void leave(int);

public:
    Server(const char* ip, const char* port, int family=AF_UNSPEC, int socktype=SOCK_STREAM);
    void start(int backlog=20);
    Socket& get_socket(void);

    static bool exit;
};

#endif //SERVER_H
