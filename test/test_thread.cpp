#include "thread.h"

#include <stdio.h>
#include <unistd.h>

void* run (void* arg)
{
    Thread::mutex_lock(*(int*)arg);
    printf ("Hola, soy el thread, recibí: %d\n", *((int*)arg));
    (*(int*)arg) *= 5;
    return NULL;
}

void test1 (void)
{
    int thread_arg[5] = {0,1,2,3,4};

    Thread th[5];

    Thread::create_mutex(5, true);

    for (int i = 0; i < 5; i++)
    {
        th[i].create(run, (void *)&thread_arg[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        Thread::mutex_unlock(i);
        sleep(5);
        th[i].join();
        printf("Valor del thread: %d\n", thread_arg[i]);
    }
    
    printf ("El thread regresó\n");
}

/*int main(void)
{
    test1();
    return 0;
}*/