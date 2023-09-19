#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdio.h>
#include "tools.h"
#include "sig.h"
#include <stdexcept>

class Thread {
private:
  pthread_t id;

public:
  Thread (void*(*run)(void *), void* args=NULL, bool detached=false );
  Thread(void);
  int join(void);
  int detach(void);
  int create (void* (*run)(void*), void* args=NULL, bool detached=false);
  int send_signal(int signal);
};

#endif //THREAD_H