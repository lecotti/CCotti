#include "server.h"
#include "client.h"

#include <unistd.h>

void handler (Client& client, char* ip, int port)
{
    char msg[20];
    printf("Soy el server: me conecté con IP: %s, port: %d\n", ip, port);
    client.recv(msg, 20);
    printf("recibi del client: %s\n", msg);
}


/*int main (void)
{
    if (!fork())
    {
        Client client("::1", "3031");
        client.send( (void*) "Hola", 5);
    }

    else
    {
        //Server server("::1", "3031", handler);
    }
    
}*/