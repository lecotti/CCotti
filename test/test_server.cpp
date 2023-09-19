#include "test_server.h"

/// @brief Tested: Multiple connections from same client.
TEST (ServerTest, BasicConnectionIPv4) {
    if (!fork()) {
        // Client
        msg_t msg;
        while(!Socket::is_listening("localhost", "3000"));
        Socket socket("localhost", "3000");
        msg.number = 1;
        strcpy(msg.text, "first");
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_STREQ(msg.text, "echo: first");
        strcpy(msg.text, "second");
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_STREQ(msg.text, "echo: second");
        strcpy(msg.text, "exit");
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_STREQ(msg.text, "echo: exit");
        socket.close();
    } else {
        // Host
        EchoServer server("localhost", "3000");
        server.start();
        ASSERT_EQ(wait(NULL), -1);
    }
}

/// @brief Tested: Exact same as BasicConnectionIPv4, but with an IPv6 address.
TEST (ServerTest, BasicConnectionIPv6) {
    if (!fork()) {
        // Client
        msg_t msg;
        while(!Socket::is_listening("::1", "3000"));
        Socket socket("::1", "3000");

        // Writing
        msg.number = 1;
        strcpy(msg.text, "first");
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
        strcpy(msg.text, "second");
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
        strcpy(msg.text, "exit");
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));

        // Reading
        ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_STREQ(msg.text, "echo: first");
        ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_STREQ(msg.text, "echo: second");
        ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_STREQ(msg.text, "echo: exit");
        socket.close();
    } else {
        // Host
        int wstatus;
        EchoServer server("::1", "3000");
        server.start();
        ASSERT_EQ(wait(NULL), -1);
    }
}

/// @brief Tested: Multiple connections from different clients.
TEST (ServerTest, MultipleClients) {
    uint8_t i;
    Sem g_sem(".", 2, true);
    g_sem = 0;
    for (i=0; i<5; i++) {
        if (!fork()) {
            // Client
            while(!Socket::is_listening("localhost", "3000"));
            Socket socket("localhost", "3000");
            msg_t msg;
            msg.number = i;
            switch(i) {
                case 0:
                    strcpy(msg.text, "first");
                    ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_STREQ(msg.text, "echo: first");
                    g_sem++;
                break;
                case 1:
                    strcpy(msg.text, "second");
                    ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_STREQ(msg.text, "echo: second");
                    g_sem++;
                break;
                case 2:
                    strcpy(msg.text, "third");
                    ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_STREQ(msg.text, "echo: third");
                    g_sem++;
                break;
                case 3:
                    strcpy(msg.text, "fourth");
                    ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_STREQ(msg.text, "echo: fourth");
                    g_sem++;
                break;
                case 4:
                    g_sem.op(-4);
                    strcpy(msg.text, "exit");
                    ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_EQ(socket.read(&msg, sizeof(msg_t)), sizeof(msg_t));
                    ASSERT_STREQ(msg.text, "echo: exit");
                break;
            }
            socket.close();
            exit(0);
        }
    }
    // Host
    EchoServer server("localhost", "3000");
    server.start();
    ASSERT_EQ(wait(NULL), -1);
}

/// @brief Tested: Connection from client closed abruptly.
TEST (ServerTest, CloseClientConnection) {
    if (!fork()) {
        // Client
        while(!Socket::is_listening("localhost", "3000"));
        Socket socket("localhost", "3000");
        socket.close();
        exit(0);
    } else {
        // Server
        ClosedConnectionServer server("localhost", "3000");
        server.start();
        ASSERT_EQ(wait(NULL), -1);
    }
}

/// @brief Tested: Connection from server closed abruptly.
TEST (ServerTest, CloseServerConnection) {
    if (!fork()) {
        EchoServer server("localhost", "3000");
        server.start();
        exit(0);
    } else {
        msg_t msg;
        msg.number = 0;
        strcpy(msg.text, "exit");
        while(!Socket::is_listening("localhost", "3000"));
        Socket socket("localhost", "3000");
        socket.write(&msg, sizeof(msg_t));
        socket.read(&msg, sizeof(msg_t));
        EXPECT_STREQ(msg.text, "echo: exit");
        wait(NULL); // SIGCHLD ignore is on the child process.
        ASSERT_FALSE(Socket::is_listening("localhost", "3000"));
        // You need two writes to detect that the socket was closed.
        // Data is buffered, first "send()" returns ok, and when you call the
        // second "send()", the broken pipe is detected.
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), sizeof(msg_t));
        ASSERT_EQ(socket.write(&msg, sizeof(msg_t)), -1);
    }
}
