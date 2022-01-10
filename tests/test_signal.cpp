#include "../headers/signal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


void handler (int signal)
{
    printf ("Llego la señal: %d\n", signal);
}


int main (void)
{
    Signal sigint(SIGINT, handler);
    Signal sigusr1(SIGUSR1, handler);

    printf("Voy a dormir 10 seg, apretá \"Ctrl + C\" para cortarme\n");

    sleep(10);

    printf( "Ahora bloquee todas las señales salvo SIGINT."
            "Proba mandarme una SIGUSR1 así: \"kill -10 %d\"\n"
            "Y fijate que el handler se ejecuta después\n", getpid());

    Signal::pause(SIGINT);

    Signal::block_signal(SIGINT);

    printf("Ahora sí, voy a dormir una siesta de 7 seg y no me podés interrumpir con SIGINT\n");

    sleep(7);

    Signal::unblock_signal(SIGINT);

    printf("Termino mi siesta, probá interrumpirme ahora\n");

    sleep(15);

    printf("Por último, voy a ignorar la señal, proba detenerme\n");

    sigint.ignore_signal();

    sleep(5);

    return 0;

}