#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdio.h>

class Thread
{
private:
  pthread_t id;
  bool detached;
  static pthread_mutex_t* mutex;
  static int mutex_qtty;

  static void destroy_mutex (void);

public:
  Thread (void*(*run)(void *), void* args=NULL, bool detached=false );

  Thread (void);

  ~Thread();

  int join(void);

  int detach(void);

  void create (void* (*run)(void*), void* args=NULL, bool detached=false);

  static void create_mutex(int qtty, bool lock=false);

  static int mutex_lock(int which=0);

  static int mutex_trylock(int which=0);

  static int mutex_unlock(int which=0);
};

#endif //THREAD_H