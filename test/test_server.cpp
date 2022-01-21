#include "server.h"
#include "socket.h"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <unistd.h>
#include "sig.h"

class CreationServer : public Server
{
protected:
    int value;

    void on_accept(Socket socket) override
    {
        int msg;
        for (int i = 0; i < 5; i++)
        {
            if ((socket >> msg) < 1)
            {
                this->value = 66;
                return;
            }

            switch (i)
            {
                case 0:
                {
                    ASSERT_EQ (msg, 1);
                    break;
                }

                case 1:
                {
                    ASSERT_EQ(msg, 2);
                    break;
                }

                case 2:
                {
                    ASSERT_EQ(msg, 3);
                    break;
                }

                case 3:
                {
                    ASSERT_EQ(msg, 4);
                    break;
                }

                case 4:
                {
                    ASSERT_EQ(msg, 5);
                    break;
                }
            }
            this->value += msg;
        }
    }

    bool on_quit(void) override
    {
        return (this->value == 15 || this->value == 66);
    }

public:
    CreationServer(const char* ip, const char* port):Server(ip, port)
    {
        this->value = 0;
    }
};

class MultiServer : public Server
{
protected:
    char value [100];
    int clients;

    void on_accept(Socket socket) override
    {
        char msg[20];
        socket.read(msg, sizeof(msg));
        switch (this->clients)
        {
            case 0:
            {
                ASSERT_STREQ (msg, "hola");
                break;
            }

            case 1:
            {
                ASSERT_STREQ(msg, "esta");
                break;
            }

            case 2:
            {
                ASSERT_STREQ(msg, "es");
                break;
            }

            case 3:
            {
                ASSERT_STREQ(msg, "una");
                break;
            }

            case 4:
            {
                ASSERT_STREQ(msg, "prueba");
                break;
            }
        }
        this->clients++;
        strcat(this->value, msg);
    }

    bool on_quit(void) override
    {
        return (strcmp(this->value, "holaestaesunaprueba") == 0);
    }

public:
    MultiServer(const char* ip, const char* port):Server(ip, port)
    {
        strcpy(this->value, "");
        this->clients = 0;
    }
};


/******************************************************************************
 * Being tested: All.
 * 
 * Expected result: The server should be able to receive five connections from
 * the same client.
 *****************************************************************************/
TEST (ServerTest, Creation)
{
    if (!fork())    //Children
    {
        sleep(1);
        Socket socket("localhost", "3000");
        socket << 1;
        socket << 2;
        socket << 3;
        socket << 4;
        socket << 5; 
    }

    else    //Father
    {
        CreationServer server("localhost", "3000");
        server.start();
    }
}

/******************************************************************************
 * Being tested: Ipv6 connection.
 * 
 * Expected result: The server should be able to receive five connections from
 * the same client.
 *****************************************************************************/
TEST (ServerTest, CreationWithIPv6)
{
    if (!fork())    //Children
    {
        sleep(1);
        Socket socket("::1", "3000");
        socket << 1;
        socket << 2;
        socket << 3;
        socket << 4;
        socket << 5;     
    }

    else    //Father
    {
        CreationServer server("::1", "3000");
        server.start();
    }
}

/******************************************************************************
 * Being tested: Reception of multiple clients.
 * 
 * Expected result: The server should be able to handle five different cliets.
 *****************************************************************************/
TEST (ServerTest, MultipleClients)
{
    if (!fork())    //Children
    {
        sleep(1);
        Socket socket0("localhost", "3000");
        socket0 << "hola";
        Socket socket1("localhost", "3000");
        socket1 << "esta";
        Socket socket2("localhost", "3000");
        socket2 << "es";
        Socket socket3("localhost", "3000");
        socket3 << "una";
        Socket socket4("localhost", "3000");
        socket4 << "prueba";
    }

    else    //Father
    {
        MultiServer server("localhost", "3000");
        server.start();
    }
}

/******************************************************************************
 * Being tested: The client socket is closed abrutly.
 * 
 * Expected result: The server should detect the closed connection.
 *****************************************************************************/
TEST (ServerTest, CloseClientConnection)
{
    if (!fork())    //Children
    {
        sleep(1);
        Socket socket("localhost", "3000");
    }

    else    //Father
    {
        CreationServer server("localhost", "3000");
        server.start();
    }
}

/******************************************************************************
 * Being tested: The server socket is closed abruptly.
 * 
 * Expected result: The client should raise a SIGPIPE after writing two times
 * after the closing of the socket.
 *****************************************************************************/
TEST (ServerTest, CloseServerConnection)
{
    if (!fork())    //Children
    {
        CreationServer server("localhost", "3000");
        server.start();
    }

    else    //Father
    {
        int msg = 0;
        sleep(1);
        Socket socket("localhost", "3000");
        socket << 1;
        socket << 2;
        socket << 3;
        socket << 4;
        socket << 5;
        socket << 6;
        EXPECT_EXIT(socket << 7, testing::KilledBySignal(SIGPIPE), "");
        wait(NULL);
    }
}
