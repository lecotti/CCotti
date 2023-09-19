#include "server.h"
#include "socket.h"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <unistd.h>
#include "sig.h"
#include "sem.h"

typedef struct msg_t {
    char text[50];
    int number;
} msg_t;

class EchoServer: public Server {
protected:
    void on_accept(Socket& socket) override {
        msg_t msg_read;
        msg_t msg_echo;
        strcpy(msg_read.text, "");
        while(socket.read(&msg_read, sizeof(msg_t)) > 0) {
            strcpy(msg_echo.text, "echo: ");
            msg_echo.number = msg_read.number;
            strcat(msg_echo.text, msg_read.text);   // "echo: <msg_read.text>"
            ASSERT_EQ(socket.write(&msg_echo, sizeof(msg_t)), sizeof(msg_t));
            if (strcmp(msg_read.text, "exit") == 0) {
                Signal::kill(getppid(), SIGINT);
                return;
            }
        }
    }
public:
    EchoServer(const char* ip, const char* port): Server(ip, port) {}
};

class ClosedConnectionServer: public Server {
protected:
    void on_accept(Socket& socket) override {
        int buff;
        ASSERT_EQ(socket.read(&buff, sizeof(buff)), 0);
        Signal::kill(getppid(), SIGINT);
    }
public:
    ClosedConnectionServer(const char* ip, const char* port): Server(ip, port) {}
};
