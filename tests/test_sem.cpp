#include "../headers/shared_memory.h"
#include "../headers/sem.h"
#include "../headers/tools.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int main (void)
{
    Sem sem(5, ".", 2);
    sem - 0;
    sem - 1;
    sem - 2;
    int pid;
    int i;

    printf ("Los semáforos 0, 1 y 2 están desactivados, el resto debería seguir andando.\n");

    for (i = 0; i < 5; i++)
    {
        if ( (pid = fork())  == 0 )
        {
            printf ("Bebe %d, y el sem vale %d\n", i, sem.get_value(i));
            sem - i;
            printf ("hola papa, soy el hijo %d, y el sem vale %d\n", i, sem.get_value());
            break;
        }
    }

    if (pid)
    {
        printf("Soy el padre, voy a dormir 10 segundos, y después voy a habilitar el semáforo, ready?\n");

        sleep(10);

        sem++;

        for (int i = 0; i < 5; i++)
        {
            wait(NULL);
        }

        sem.free();
        
    }
}