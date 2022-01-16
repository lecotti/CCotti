#include "thread.h"

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
 *  @return:    Create retorna "0" en éxito, "-1" en error, y el constructor
 *              levanta una excepción de valor std::runtime_error
 * ***************************************************************************/
Thread::Thread(void* (*run)(void*), void* args, bool detached)
{
    if (this->create(run, args, detached) != 0)
    {
        throw(std::runtime_error("create"));
    }
}

int Thread::create (void* (*run)(void*), void* args, bool detached)
{
    if (pthread_create(&(this->id), NULL, run, args) != 0)
    {
        perror(ERROR("Couldn't create thread with pthread_create.\n"));
        return -1;
    }

    if (detached)
    {
        return this->detach();
    }

    return 0;
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
    if (pthread_join(id, NULL) != 0)
    {
        perror(WARNING("Couldn't join thread with pthread_join. Maybe is detached?\n"));
        return -1;
    }

    return 0;
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
    if (pthread_detach(this->id) != 0)
    {
        perror(ERROR("Couldn't detach from thread.\n"));
        return -1;
    }
    return 0;
}

/******************************************************************************
 *  @brief:     Le envia una señal al thread en cuestión.
 * 
 *  @arg:       signal: señal a enviar.
 * 
 *  @return:    "0" en éxito, -1 en error.
 * ***************************************************************************/
int Thread::kill(int signal)
{
    return Signal::kill(this->id, signal);
}



