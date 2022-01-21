#include "mutex.h"

/******************************************************************************
 *  @brief:     Crea una variable mutex.
 * 
 *  @arg:       Void.
 * 
 *  @return:    None.
 * ***************************************************************************/
Mutex::Mutex()
{
    this->mutex = PTHREAD_MUTEX_INITIALIZER;
}

/******************************************************************************
 *  @brief:     Reserva el mutex. Si ya estaba reservado por otro thread, el 
 *              thread que llame a esta función quedará bloqueado hasta que 
 *              el mutex sea liberado.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error. 
 * ***************************************************************************/
int Mutex::lock(void)
{
    if (pthread_mutex_lock(&(this->mutex)) != 0)
    {
        perror(ERROR("Trying to lock mutex with pthread_mutex_lock.\n"));
        return -1;
    }

    return 0;
}

/******************************************************************************
 *  @brief:     Libera una variable mutex bloqueada.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error. 
 * ***************************************************************************/
int Mutex::unlock(void)
{
    if (pthread_mutex_unlock(&(this->mutex)) != 0)
    {
        perror(ERROR("Trying to unlock the mutex with pthread_mutex_unlock.\n"));
        return -1;
    }
    return 0;
}

/******************************************************************************
 *  @brief:     Intenta reservar una variable mutex de forma no bloqueante.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito (reservó el mutex), "-1" si el mutex estaba ocupado y 
 *              no pudo reservarlo, o si hubo algún error.
 * ***************************************************************************/
int Mutex::trylock(void)
{
    if (pthread_mutex_trylock(&(this->mutex)) != 0)
    {
        return -1;
    }
    return 0;
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
 *  @return:    None.
 * ***************************************************************************/
Mutex::~Mutex(void)
{
    pthread_mutex_destroy(&(this->mutex));
}
