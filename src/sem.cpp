#include "sem.h"

/******************************************************************************
 *  @brief:     Crea un semáforo con valor inicial 1. La forma de operarlo es:
 *              para bloquear sem--. Para liberar sem++.
 * 
 *  @arg:       init_value: valor inicial de los semáforos.
 * 
 *              path: path a un archivo. Necesario para identificar inequívoca-
 *              mente al set de semáforos.
 * 
 *              id: Identifica inequívocamente al set de semáforos.
 * 
 *  @return:    Throws "std::runtime_error" en caso de error.
 * ***************************************************************************/
Sem::Sem(const char* path, int id, bool create): sem_qtty(sem_qtty)
{
    key_t key;
    
    if ( (key = ftok(path, id) ) == -1)
    {
        perror(ERROR("Couldn't get semaphore with ftok.\n"));
        throw(std::runtime_error("Ftok"));
    }

    if (create)
    {
        if ( (this->semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666) ) == -1 )
        {
            perror(ERROR("Couldn't create semaphore with semget.\n"));
            throw(std::runtime_error("Semget"));
        }

        if (this->set(1) != 0)
        {
            perror(ERROR("Setting sem values.\n"));
            throw(std::runtime_error("Set_value"));
        }
    }

    else
    {
        if ( (this->semid = semget(key, 0, 0) ) == -1 )
        {
            perror(ERROR("Couldn't connect to existing semaphore.\n"));
            throw(std::runtime_error("Semget"));
        }
    } 
}

/******************************************************************************
 *  @brief:     Chequea si el semáforo existo
 * 
 *  @arg:       path: path a un archivo. Necesario para identificar inequívoca-
 *              mente al set de semáforos.
 * 
 *              id: Identifica inequívocamente al set de semáforos.
 * 
 *  @return:    Throws "std::runtime_error" en caso de error.
 * ***************************************************************************/
bool Sem::exists(const char* path, int id)
{
    key_t key;
    if ( (key = ftok(path, id) ) == -1)
    {
        return false;
    }

    if ( (semget(key, 0, 0) ) == -1 )
    {
        return false;
    }

    return true;

}

/******************************************************************************
 *  @brief:     Setea el valor del semáforo a un valor específico.
 * 
 *  @arg:       value: El valor que tomará el semáforo. 
 * 
 *  @return:    "0" en éxtio, "-1" en error.
 * ***************************************************************************/
int Sem::set (int value)
{
    return semctl(this->semid, 0, SETVAL, value);
}

/******************************************************************************
 *  @brief:     Devuelve el valor que tiene el semáforo.
 * 
 *  @arg:       Void.
 * 
 *  @return:    EL valor que tiene el semáforo, o "-1" en error.
 * ***************************************************************************/
int Sem::get(void) const
{
    return semctl(this->semid, 0, GETVAL);
}

/******************************************************************************
 *  @brief:     Realiza una operación sobre UNO de los semáforos del set. Hay
 *              tres operaciones posibles:
 *              
 *              op = 0 : El proceso bloquea hasta que el valor del semáforo sea
 *              exactamente cero.
 * 
 *              op > 0 : Le suma el valor al semáforo.
 * 
 *              op < 0 : Si |op| <= |semval|, entonces le resta el valor. Si
 *              |op| > |semval|, entonces bloquea hasta que pueda restarlo. El
 *              valor de "semval" no puede ser negativo.
 * 
 *  @arg:       op: EL valor de la operación.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::op (int op)
{
    struct sembuf sop;

    sop.sem_num = 0;
    sop.sem_op = op;
    sop.sem_flg = 0;

    return semop(this->semid, &sop, 1);
}


/******************************************************************************
 *  @brief:     Destruye el semáforo, liberando recursos.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::free(void)
{
    return semctl(this->semid, 0, IPC_RMID);
}

/******************************************************************************
 *  @brief:     Le suma "+1" al semáforo.
 * 
 *  @arg:       Void.
 * 
 *  @return:    El valor que le queda al semáforos en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator++ (int)
{
    this->op(1);
    return this->get();
}

int Sem::operator++ ()
{
    this->op(1);
    return this->get();
}

/******************************************************************************
 *  @brief:     Le resta "1" al semáforo.
 * 
 *  @arg:       Void.
 * 
 *  @return:    El valor que le queda al semáforo en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator-- (int)
{
    this->op(-1);
    return this->get();
}

int Sem::operator-- ()
{
    this->op(1);
    return this->get();
}

/******************************************************************************
 *  @brief:     Setea el valor del semáforo.
 * 
 *  @arg:       El valor que tendŕa el semáforo.
 * 
 *  @return:    Void.
 * ***************************************************************************/
int Sem::operator= (int a)
{
    this->set(a);
    return this->get();
}

/******************************************************************************
 *  @brief:     Le suma una cantidad arbitraria al semáforo.
 * 
 *  @arg:       a: el número a sumarle.
 * 
 *  @return:    El valor del semáforo en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator+ (int a)
{
    this->op(a);
    return this->get();
}

/******************************************************************************
 *  @brief:     Le resta una cantidad al semáforo.
 * 
 *  @arg:       a: el número a restarle
 * 
 *  @return:    El valor del semáforo en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator- (int a)
{
    this->op(-a);
    return this->get();
}

 