#include "sig.h"

/// @brief Sets a signal handler for certain "signal"
/// @param signal Signal number
/// @param signal_handler Function to be called when the signal is received. Can be:
///  * void(*foo)(int) function pointer.
///  * SIG_IGN to ignore the signal.
///  * SIG_DFL to use the default handler.
/// @param flags Handler flags ("0" by default).
/// @param signals_blocked_in_handler Vector with signal numbers to block while
///  the handler is being executed. (NULL by default).
/// @param size Size of the "signals_blocked_in_handler" vector ("0" by default).
/// @return "0" on success, "-1" on error.
int Signal::set_handler (int signal, void(*signal_handler)(int), int flags, int* signals_blocked_in_handler, int size) {
    sigset_t mask;
    struct sigaction sa;
    if (sigemptyset(&mask) != 0) {
        perror(ERROR("sigemptyset in Signal::set_handler"));
        return -1;
    }
    for (int i = 0; i < size; i++) {
        if (sigaddset(&mask, signals_blocked_in_handler[i]) != 0) {
            perror(ERROR("sigaddset in Signal::set_handler"));
            return -1;
        }
    }
    sa.sa_mask = mask;
    sa.sa_handler = signal_handler;
    sa.sa_flags = flags;
    if (sigaction(signal, &sa, NULL) != 0) {
        perror("sigaction in Signal::set_handler");
        return -1;
    }
    return 0;
}

/// @brief Ignores a signal. It will be received by the proccess and marked as
///  attended, but no processing will be made (SIG_IGN).
/// @param signal Signal number.
/// @return "0" on success, "-1" on error.
int Signal::ignore(int signal) {
    return set_handler(signal, SIG_IGN, 0);
}

/// @brief Sets the signal handler to its default handler (SIG_DFL).
/// @param signal Signal number.
/// @return "0" on success, "-1" on error.
int Signal::set_default_handler(int signal) {
    return set_handler(signal, SIG_DFL, 0);
}

/// @brief Blocks a signal. The signal will be pending until unblocked, if received.
/// @param signal Signal number.
/// @return "0" on success, "-1" on error.
int Signal::block(int signal) {
    sigset_t mask;
    if (sigemptyset(&mask) != 0) {
        perror(ERROR("sigemptyset in Signal::block"));
        return -1;
    }
    if (sigaddset(&mask, signal) != 0) {
        perror(ERROR("sigaddset in Signal::block"));
        return -1;
    }
    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        perror((ERROR("pthread_sigmask in Signal::block")));
        return -1;
    }
    return 0;
}

/// @brief Unblocks a previously blocked signal
/// @param signal Signal number
/// @return "0" on success, "-1" on error.
int Signal::unblock(int signal) {
    sigset_t mask;
    if (sigemptyset(&mask) != 0) {
        perror(ERROR("sigemptyset in Signal::unblock"));
        return -1;
    }
    if (sigaddset( &mask, signal) != 0) {
        perror(ERROR("sigaddset in Signal::unblock"));
        return -1;
    }
    if (pthread_sigmask(SIG_UNBLOCK, &mask, NULL) != 0) {
        perror((ERROR("pthread_sigmask in Signal::unblock")));
        return -1;
    }
    return 0;
}

/// @brief Unblocks all signal, initializes the signal mask.
/// @return "0" on success, "-1" on error.
int Signal::unblock_all(void) {
    sigset_t mask;
    if (sigemptyset(&mask) != 0) {
        perror(ERROR("sigemptyset in Signal::unblock_all"));
        return -1;
    }
    if (pthread_sigmask(SIG_SETMASK, &mask, NULL) != 0) {
        perror((ERROR("pthread_sigmask in Signal::unblock_all")));
        return -1;
    }
    return 0;
}

/// @brief Sends a signal to a process.
/// @param pid Process ID.
/// @param signal Signal number
/// @return "0" on success, "-1" on error.
int Signal::kill (pid_t pid, int signal) {
    if (::kill(pid, signal) != 0) {
        perror(ERROR("kill in Signal::kill"));
        return -1;
    }
    return 0;
}

/// @brief Sends a signal to a thread.
/// @param pid Thread ID.
/// @param signal Signal number
/// @return "0" on success, "-1" on error.
int Signal::kill (pthread_t thread_id, int signal) {
    if (pthread_kill(thread_id, signal) != 0) {
        perror(ERROR("pthread_kill in Signal::kill"));
        return -1;
    }
    return 0;
}

/// @brief Blocks until the "signal" is received, and then executes its
///  associated handler.
/// @param signal Signal number.
/// @return "0" on success, "-1" on error.
int Signal::wait (int signal) {
    sigset_t mask;
    if (sigfillset(&mask) != 0) {
        perror(ERROR("sigfillset in Signal::wait"));
        return -1;
    }
    if (sigdelset(&mask, signal) != 0) {
        perror(ERROR("sigdelset in Signal::wait"));
        return -1;
    }
    sigsuspend(&mask); // Always returns -1 for signal interruption.
    return 0;
}

/// @brief Blocks until the "signal" is received. But no handler is executed.
/// @param signal Signal number.
/// @return "0" on success, "-1" on error.
int Signal::wait_and_ignore (int signal) {
    sigset_t mask;
    int sig_return = 0;
    if (sigemptyset(&mask) != 0) {
        perror(ERROR("sigemptyset in Signal::wait_and_ignore"));
        return -1;
    }
    if (sigaddset(&mask, signal) != 0) {
        perror(ERROR("sigaddset in Signal::wait_and_ignore"));
        return -1;
    }
    if (sigwait(&mask, &sig_return) != 0) {
        perror(ERROR("sigwait in Signal::wait_and_ignore"));
        return -1;
    }
    return 0;
}

/******************************************************************************
 * Timer with SIGALRM
******************************************************************************/

/// @brief Sends SIGALRM to this same thread after "msec" milliseconds have
///  passed, only once. Always succeeds.
/// @param msec Time in miliseconds.
/// @return "0" on success, "-1" on error.
void Signal::set_timer_single_shot(time_t msec) {
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = msec / 1000;
    timer.it_value.tv_usec = (msec*1000) % 1000000;
    setitimer(ITIMER_REAL, &timer, NULL);
}

/// @brief Sends SIGALRM to this same thread after "msec" milliseconds have
///  passed, periodically. Always succeeds.
/// @param msec Time in miliseconds.
void Signal::set_timer_periodic(time_t msec) {
    struct itimerval timer;
    timer.it_interval.tv_sec = msec / 1000;
    timer.it_interval.tv_usec = (msec*1000) % 1000000;
    timer.it_value.tv_sec = timer.it_interval.tv_sec;
    timer.it_value.tv_usec = timer.it_interval.tv_usec;
    setitimer(ITIMER_REAL, &timer, NULL);
}

/// @brief Disarm timer. Always succeeds.
void Signal::unset_timer(void) {
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

/// @brief Return amount of time remaining in the timer, in milliseconds.
time_t Signal::get_timer_time(void) {
    struct itimerval timer;
    getitimer(ITIMER_REAL, &timer);
    return (timer.it_value.tv_sec*1000 + timer.it_value.tv_usec/1000);
}
