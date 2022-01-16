#ifndef SIG_H
#define SIG_H

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include "tools.h"
#include <unistd.h>

namespace Signal
{

int set_handler (int signal, void(*signal_handler)(int), int flags=0, int* to_block=NULL, int size=0);
int ignore(int signal);
int set_default_handler(int signal);
int block(int signal);
int unblock(int signal);
int unblock_all(void);
int kill (pid_t pid, int signal);
int kill (pthread_t thread_id, int signal);
int set_alarm(int time);
int wait_and_handle (int signal);
int wait_and_continue (int signal);


} // namespace Signal

#endif // SIG_H