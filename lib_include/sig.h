#ifndef SIG_H
#define SIG_H

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include "tools.h"
#include <unistd.h>
#include <sys/time.h>

namespace Signal {
    int set_handler (int signal, void(*signal_handler)(int), int flags=0, int* signals_blocked_in_handler=NULL, int size=0);
    int ignore(int signal);
    int set_default_handler(int signal);
    int block(int signal);
    int unblock(int signal);
    int unblock_all(void);
    int kill (pid_t pid, int signal);
    int kill (pthread_t thread_id, int signal);
    int wait (int signal);
    int wait_and_ignore (int signal);
    void set_timer_single_shot(time_t msec);
    void set_timer_periodic(time_t msec);
    void unset_timer(void);
    time_t get_timer_time(void);
} // namespace Signal

#endif // SIG_H