#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include "../headers/sem.h"

class Signal
{
private:
    static sigset_t sig_mask;
    static bool mask_initialized;

    struct sigaction sa;
    int signal;

    int update(void);

public:

    Signal (int signal, void(*signal_handler)(int), int flags = 0);

    int change_handler(void(*signal_handler)(int));

    int ignore_signal(void);    

    int restore_default_handler(void);

    static int block_signal(int signal);

    static int unblock_signal(int signal);

    static int unblock_all (void);

    static int kill (pid_t pid, int signal); 

    static void pause (int signal);

};

#endif //SIGNAL_H