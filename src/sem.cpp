#include "../headers/sem.h"

/******************************************************************************
 *  @brief:     Crea varios semáforos con un valor inicial.
 * 
 *  @arg:       sem_qtty: Cantidad de semáforos a crear.
 * 
 *              init_value: valor inicial de los semáforos.
 * 
 *              path: path a un archivo. Necesario para identificar inequívoca-
 *              mente al set de semáforos.
 * 
 *              id: Identifica inequívocamente al set de semáforos.
 * 
 *  @return:    None.
 * ***************************************************************************/
Sem::Sem(int sem_qtty, const char* path, int id): sem_qtty(sem_qtty)
{
    key_t key;
    
    if ( (key = ftok(path, id) ) != -1)
    {
        if (sem_qtty)
        {
            if ( (this->semid = semget (key, sem_qtty, IPC_CREAT | 0666) ) == -1 )
            {
                perror(RED("Semget.\n"));
                return;
            }

            if (this->set_value(1) == -1)
            {
                perror(RED("Setting sem values.\n"));
                return;
            }
        }

        else
        {
            if ( (this->semid = semget (key, 0, 0) ) == -1 )
            {
                perror(RED("Semget.\n"));
                return;
            }
        }
        
    }

    else
    {
        perror(RED("Ftok.\n"));
        return;
    }
}

/******************************************************************************
 *  @brief:     Setea el valor de TODOS los semáforos a un valor específico.
 * 
 *  @arg:       value: El valor que tomará el semáforo. 
 * 
 *              semnum: El número de semáforo.
 * 
 *  @return:    "0" en éxtio, "-1" en error.
 * ***************************************************************************/
int Sem::set_value (int value)
{
    unsigned short array [this->sem_qtty];

    for (int i = 0; i < this->sem_qtty; i++)
    {
        array[i] = value;
    }

    return semctl(this->semid, 0, SETALL, array);
}

/******************************************************************************
 *  @brief:     Setea el valor de UNO de los semáforos a un valor específico.
 * 
 *  @arg:       value: El valor que tomará el semáforo. 
 * 
 *              semnum: El número de semáforo.
 * 
 *  @return:    "0" en éxtio, "-1" en error.
 * ***************************************************************************/
int Sem::set_value (int value, int semnum)
{
    return semctl(this->semid, semnum, SETVAL, value);
}

/******************************************************************************
 *  @brief:     Devuelve el valor que tiene el semáforo.
 * 
 *  @arg:       semnum: el número de semáforo.
 * 
 *  @return:    EL valor que tiene el semáforo, o "-1" en error.
 * ***************************************************************************/
int Sem::get_value (int semnum)
{
    return semctl(this->semid, semnum, GETVAL);
}

/******************************************************************************
 *  @brief:     Realiza una operación sobre TODOS los semáforos. Hay tres op. definidas:
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
    struct sembuf sop[this->sem_qtty];

    for (int i = 0; i < this->sem_qtty; i++)
    {
        sop[i].sem_num = i;
        sop[i].sem_op = op;
        sop[i].sem_flg = 0;
    }

    return semop(this->semid, sop, this->sem_qtty);
}

/******************************************************************************
 *  @brief:     Realiza una operación sobre UNO de los semáforos del set.
 * 
 *  @arg:       op: EL valor de la operación.
 * 
 *              semnum: El núemro de semáforo.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::op (int op, int semnum)
{
    struct sembuf sop;

    sop.sem_num = semnum;
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
 *  @brief:     Le suma "+1" a TODOS los semáforos.
 * 
 *  @arg:       Void.
 * 
 *  @return:    El valor que le queda a los semáforos en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator++ (int)
{
    this->op(1);
    return this->get_value();
}

int Sem::operator++ ()
{
    this->op(1);
    return this->get_value();
}

/******************************************************************************
 *  @brief:     Le resta "1" a TODOS los semáforos.
 * 
 *  @arg:       Void.
 * 
 *  @return:    El valor que le queda a los semáforos en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator-- (int)
{
    this->op(-1);
    return this->get_value();
}

int Sem::operator-- ()
{
    this->op(1);
    return this->get_value();
}

/******************************************************************************
 *  @brief:     Setea el valor de TODOS los semáforos.
 * 
 *  @arg:       Void.
 * 
 *  @return:    Void.
 * ***************************************************************************/
void Sem::operator= (int a)
{
    this->set_value(a);
}

/******************************************************************************
 *  @brief:     Le suma "+1" al semáforo "a" del set.
 * 
 *  @arg:       a: el número de semáforo dentro del set.
 * 
 *  @return:    El valor del semáforo en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator+ (int a)
{
    this->op(1, a);
    return this->get_value(a);
}

/******************************************************************************
 *  @brief:     Le resta "1" al semáforo "a" del set.
 * 
 *  @arg:       a: el número de semáforo dentro del set.
 * 
 *  @return:    El valor del semáforo en éxito, "-1" en error.
 * ***************************************************************************/
int Sem::operator- (int a)
{
    this->op(-1, a);
    return this->get_value(a);
}

 