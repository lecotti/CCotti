#include "thread.h"

/// @brief Creates a thread and executes it right away.
/// @param run Function to execute. Can receive and return any type, but must
///  be defined with void*.
/// @param args Arguments to be passed to the thread function when run. You
///  should cast it to a void*.
/// @param detached Whether the Thread runs detached or no (default = "false").
/// @return Throws std::runtime_error in case of error.
Thread::Thread(void* (*run)(void*), void* args, bool detached) {
    if (this->create(run, args, detached) != 0) {
        throw(std::runtime_error("create"));
    }
}

/// @brief Empty constructor. Create thread with Thread::create() afterwards.
Thread::Thread(void) {}

/// @brief Creates a thread that was not initialized with the constructor.
/// @return "0" on success, "-1" on error.
int Thread::create(void* (*run)(void*), void* args, bool detached) {
    if (pthread_create(&(this->id), NULL, run, args) != 0) {
        perror(ERROR("pthread_create in Thread::create"));
        return -1;
    }
    if (detached) {
        return this->detach();
    }
    return 0;
}

/// @brief Waits for the thread to end, in a blocking manner.
/// @return "0" on success, "-1" on error.
int Thread::join(void) {
    if (pthread_join(id, NULL) != 0) {
        perror(WARNING("pthread_join in Thread::join. Maybe it's detached"));
        return -1;
    }
    return 0;
}

/// @brief The thread will run independently from the parent. You should not
///  join a detached thread.
/// @return "0" on success, "-1" on error.
int Thread::detach(void) {
    if (pthread_detach(this->id) != 0) {
        perror(ERROR("pthread_detatch in Thread::detach"));
        return -1;
    }
    return 0;
}

/// @brief Sends a signal to the Thread.
/// @param signal Signal number
/// @return "0" on success, "-1" on error.
int Thread::send_signal(int signal) {
    return Signal::kill(this->id, signal);
}



