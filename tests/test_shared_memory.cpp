#include "../headers/shared_memory.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (void)
{
    printf ("sizeof(float) = %ld\n", sizeof(double));
    float a[5];
    int pid;

    SharedMemory<float> sh(6, ".", 2);
    Sem sem(1, ".", 2);

    for (int i = 0; i < 6; i++)
    {
        sh[i] = i;
    }

    sh.read(a,6);

    for (int i = 0; i < 6; i++)
    {
        printf ("%d: %f\n", i, a[i]);
    }

    printf ("Voy a poner a 30 hijos a la vez a escribir en la Shared Memory, veamos como va eso\n");
    
    
    for (int i = 0; i < 30; i++)
    {
        if ( (pid = fork()) == 0)
        {
            
            sem--;
            sh.write(sh.read(0) + 1, 0);
            sem++;
            return 0;
        }
    }

    for (int i = 0; i < 30; i++)
    {
        wait(NULL);
    }

    printf("Ahora el valor que modificaron los 30 hijos vale: %f\n", sh[0]);

    sh.free();
    return 0;
}