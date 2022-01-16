#include "thread.h"

/******************************************************************************
 *  STATIC VARIABLES
 * ***************************************************************************/
pthread_mutex_t* Thread::mutex;

int Thread::mutex_qtty = 0;

/******************************************************************************
 *  @brief:     Crea un thread que ejecutará la función "run".
 * 
 *  @arg:       run: La función a ejecutar. Puede recibir como argumentos un
 *              puntero a <arg_t> y devolver un puntero a <arg_t>.
 * 
 *              args: Un puntero al argumento que recibe el thread. Por ejemplo,
 *              si se crea en el programa un "int a = 5"; le pasarías a esta
 *              función un "(void *) &a"; en el thread accederías como
 *              "*(int *)a_th"; y si se modifica el valor en el thread de "a_th",
 *              se modifica el valor de "a".
 * 
 *              detached: Si el thread correra detached o no.
 * 
 *  @return:    None.
 * ***************************************************************************/
Thread::Thread(void* (*run)(void*), void* args, bool detached): detached(detached)
{
  this->create(run, args, detached);
}

void Thread::create (void* (*run)(void*), void* args, bool detached)
{
  if (pthread_create(&(this->id), NULL, run, args) == 0)
  {
    if (detached)
    {
      this->detach();
    }
  }

  else
  {
    perror(RED("No se pudo crear el thread.\n"));
  }
}

/******************************************************************************
 *  @brief:     Constructor vacío. Para iniciar el thread, llamar a Thread::create().
 * 
 *  @arg:       Void.
 * 
 *  @return:    None.
 * ***************************************************************************/
Thread::Thread(void) {}

/******************************************************************************
 *  @brief:     Espera de forma bloqueante a que termine el thread.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Thread::join(void)
{
  if (!this->detached)
  {
    return pthread_join(id, NULL);

  }

  return -1;
}

/******************************************************************************
 *  @brief:     "Detached", el thread correra en modo daemon, independiente del
 *              thread que le dio origen.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, -1 en error.
 * ***************************************************************************/
int Thread::detach(void)
{
  this->detached = true;
  return pthread_detach(this->id);
}

/******************************************************************************
 *  @brief:     Crea "qtty" variables mutex, desde [0 : qtty -1]. Está función
 *              sobrescribe cualquier variable mutex que existiera previamente.
 * 
 *  @arg:       qtty: cuántas variables crear
 * 
 *              lock: Si querés reservarlos apenás lo creas.
 * 
 *  @return:    Void
 * ***************************************************************************/
void Thread::create_mutex(int qtty, bool lock)
{
  if (qtty)
  {
    Thread::destroy_mutex();
    Thread::mutex = new pthread_mutex_t[qtty];
    Thread::mutex_qtty = qtty;

    for (int i = 0; i < qtty; i++)
    {
      Thread::mutex[i] = PTHREAD_MUTEX_INITIALIZER;
      if (lock)
      {
        Thread::mutex_lock(i);
      }
    }
  }
}

/******************************************************************************
 *  @brief:     Reserva el mutex. Si ya estaba reservado por otro thread, el 
 *              thread que llame a esta función quedará bloqueado hasta que 
 *              el mutex sea liberado.
 * 
 *  @arg:       which: Qué mutex reservar.
 * 
 *  @return:    "0" en éxito, "-1" en error. 
 * ***************************************************************************/
int Thread::mutex_lock(int which)
{
  if (which < Thread::mutex_qtty && which > 0)
  {
    return pthread_mutex_lock(&Thread::mutex[which]);
  }

  else
  {
    return -1;
  }
}

/******************************************************************************
 *  @brief:     Libera una variable mutex bloqueada.
 * 
 *  @arg:       which: que variable mutex reservar.
 * 
 *  @return:    "0" en éxito, "-1" en error (el mutex no pertenecía al thread). 
 * ***************************************************************************/
int Thread::mutex_unlock(int which)
{
  if (which < Thread::mutex_qtty && which > 0)
  {
    return pthread_mutex_unlock(&Thread::mutex[which]);
  }

  else
  {
    return -1;
  }
}

/******************************************************************************
 *  @brief:     Intenta reservar una variable mutex de forma no bloqueante.
 * 
 *  @arg:       which: que variable mutex reservar.
 * 
 *  @return:    "0" en éxito, "-1" en error (el mutex estaba ocupado o mal
 *              inicializado). 
 * ***************************************************************************/
int Thread::mutex_trylock(int which)
{
  if (which < Thread::mutex_qtty && which > 0)
  {
    return pthread_mutex_trylock(&Thread::mutex[which]);
  }

  else
  {
    return -1;
  }
}

/******************************************************************************
 *    MÉTODOS PRIVADOS Y DESTRUCTOR
 * ***************************************************************************/

/******************************************************************************
 *  @brief:     Destruye todas las variables mutex que existían, y libera los
 *              recursos.
 * 
 *  @arg:       Void.
 * 
 *  @return:    Void.
 * ***************************************************************************/
void Thread::destroy_mutex(void)
{
  if (Thread::mutex_qtty) //Ya existían variables, tengo que limpiarlas.
  {
    for (int i = 0; i < Thread::mutex_qtty; i++)
    {
      pthread_mutex_destroy(&Thread::mutex[i]);
    }

    Thread::mutex_qtty = 0;

    delete [] Thread::mutex;
  }
}

/******************************************************************************
 *  @brief:     Si detecta que es la última instancia, libera los recursos static.
 * 
 *  @arg:       Void.
 * 
 *  @return:    None.
 * ***************************************************************************/
Thread::~Thread()
{
  Thread::destroy_mutex();
}


