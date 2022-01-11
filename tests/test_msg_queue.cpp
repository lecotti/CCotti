#include "../headers/msg_queue.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

struct persona
{
    char nombre[20];
    int id;
};

void test1 (void)
{
    struct persona p_s;
    struct persona recibido[5];

    MsgQueue<persona> queue(".", 2);
    int pid;
    int i;

    for (i = 0; i < 5; i++)
    {
        if ( (pid = fork() == 0) )
        {
            //child
            p_s.id = i;
            strcpy(p_s.nombre, "Gonzalo");
            printf("Nombre: %s | Edad: %d\n", p_s.nombre, p_s.id);
            if (queue.send(p_s) == -1)
            {
                perror ("send.\n");
            }
            return;
        }
    }

    queue >> recibido[0];
    queue >> recibido[1];
    queue >> recibido[2];
    queue >> recibido[3];
    queue >> recibido[4];


    for (i = 0; i < 5; i++)
    {
        printf("Nombre: %s | Edad: %d\n", recibido[i].nombre, recibido[i].id);
        wait(NULL);
    }

    queue.free();
}

void test2 (void)
{
    struct persona p_s;
    struct persona p_r;

    MsgQueue<persona> queue(".", 2);
    int pid;
    int i = 0;

    //if ( (pid = fork() == 0) )
    //{
        //child
        p_s.id = i;
        strcpy(p_s.nombre, "Gonzalo");
        printf("Envio:  Nombre: %s | Edad: %d\n", p_s.nombre, p_s.id);
        queue << p_s;

    //}

    queue >> p_r;

    printf("Nombre: %s | Edad: %d\n", p_r.nombre, p_r.id);
    //wait(NULL);

    queue.free();
}

int main (void)
{
    
    test1();
    return 0;
}