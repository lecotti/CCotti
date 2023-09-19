#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <stdio.h>
#include "tools.h"

class Mutex {
private:
    pthread_mutex_t mutex;
public:
    Mutex();
    int lock(void);
    int trylock(void);
    int unlock(void);
    ~Mutex();
};

#endif // MUTEX_H