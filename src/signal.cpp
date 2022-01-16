#include "sig.h"

/******************************************************************************
 *  @brief:     Inicializa un signal_handler para cierta "signal".
 * 
 *  @arg:       signal: señal a capturar.
 * 
 *              signal_handler: callback de la señal. Usar SIG_IGN para ignorar,
 *               o SIG_DFL para el handler por defecto.
 * 
 *              flags: flags del handler.
 * 
 *              to_block: señales a bloquear durante la ejecución del handler
 *              (la señal que activa el handler está bloqueada por defecto y
 *              se activa con un flag).
 * 
 *              size: tamaño del vector.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::set_handler (int signal, void(*signal_handler)(int), int flags, int* to_block, int size)
{
    sigset_t mask;
    struct sigaction sa;

    if (sigemptyset (&mask) != 0)
    {
        perror( ERROR("Couldn't configure signal handler with sigemptyset.\n"));
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        if (sigaddset(&mask, to_block[i]) != 0)
        {
            perror( ERROR("Couldn't configure signals to block during handler with sigaddset.\n"));
            return -1;
        }
    }

    sa.sa_mask = mask;
    
    sa.sa_handler = signal_handler;
    
    sa.sa_flags = flags;
    
    if (sigaction (signal, &sa, NULL) != 0)
    {
        perror( ERROR("Coudn't configure signal handler with sigaction.\n"));
        return -1;
    }

    return 0;
}

/******************************************************************************
 *  @brief:     Ignora la señal, de modo que si es detectada no será procesada
 *              (SIG_IGN).
 * 
 *  @arg:       signal: La señal a ignorar.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::ignore(int signal)
{
    return set_handler(signal, SIG_IGN, 0);
}

/******************************************************************************
 *  @brief:     Le otorga a la señal su handler por defecto (SIG_DFL).
 * 
 *  @arg:       signal: La señal a modificar.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::set_default_handler(int signal)
{
    return set_handler(signal, SIG_DFL, 0);
}

/******************************************************************************
 *  @brief:     Bloquea una señal. Esto significa que esta no será relevada por
 *              este thread hasta que sea desbloqueada.
 * 
 *  @arg:       signal: La señal a bloquear.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::block(int signal)
{
    sigset_t mask;

    if (sigemptyset(&mask) != 0)
    {
        perror( ERROR("Couldn't block signal with sigemptyset.\n"));
        return -1;
    }

    if ( sigaddset( &mask, signal) != 0)
    {
        perror( ERROR("Couldn't block signal with sigaddset.\n"));
        return -1;
    }

    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0)
    {
        perror( ERROR("Couldn't block signal with pthead_sigmask.\n"));
        return -1;
    }
    return 0;
}

/******************************************************************************
 *  @brief:     Desbloquea una señal previamente bloqueada.
 * 
 *  @arg:       signal: la señal a desbloquear.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::unblock(int signal)
{
    sigset_t mask;

    if (sigemptyset(&mask) != 0)
    {
        perror( ERROR("Couldn't unblock signal with sigemptyset.\n"));
        return -1;
    }

    if ( sigaddset( &mask, signal) != 0)
    {
        perror( ERROR("Couldn't unblock signal with sigaddset.\n"));
        return -1;
    }
        
    if ( pthread_sigmask(SIG_UNBLOCK, &mask, NULL) != 0)
    {
        perror( ERROR("Couldn't unblock signal with pthread_sigmask.\n"));
        return -1;
    }
    return 0;
}

/******************************************************************************
 *  @brief:     Desbloquea todas las señales. Inicializa la máscara.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito. "-1" en error.
 * ***************************************************************************/
int Signal::unblock_all(void)
{
    sigset_t mask;

    if (sigemptyset(&mask ) != 0)
    {
        perror( ERROR("Couldn't unblock all signals with sigemptyset.\n"));
        return -1;
    }

    if (pthread_sigmask(SIG_SETMASK, &mask, NULL) != 0)
    {
        perror( ERROR("Couldn't unblock all signals with pthread_sigmask.\n"));
        return -1;
    }

    return 0;
}

/******************************************************************************
 *  @brief:     Envia una señal al PID o al thread.
 * 
 *  @arg:       pid | thread_id: Process | thread ID.
 * 
 *              signal: Señal a mandar.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::kill (pid_t pid, int signal)
{
    if (::kill(pid, signal) != 0)
    {
        perror( ERROR("Couldn't send signal to process.\n"));
        return -1;
    }

    return 0;
}

int Signal::kill (pthread_t thread_id, int signal)
{
    if (pthread_kill(thread_id, signal) != 0)
    {
        perror( ERROR("Couldn't send signal to thread.\n"));
        return -1;
    }

    return 0;
}


/******************************************************************************
 *  @brief:     Luego de "time" segundos, se enviará una SIGALRM al proceso que
 *              llamo a esta función. **WARNING**: no usar con sleep().
 * 
 *  @arg:       time: tiempo en segundos. Si es "0", se cancela la alarma previa.
 * 
 *  @return:    "0" si se creo una nueva alarma, "-1" en caso de error, o la
 *              cantidad de segundos hasta la próxima alarma, si se había seteado
 *              una previa.
 * ***************************************************************************/
int Signal::set_alarm(int time)
{
    if (time >= 0)
    {
        return alarm((unsigned int) time);
    }

    else
    {
        return -1;
    }
}




/******************************************************************************
 *  @brief:     Función bloqueante hasta que se reciba la "signal", momento en
 *              el que ejecuta el handler que tenga asociado.
 * 
 *  @arg:       signal: señal a esperar.
 * 
 *  @return:    "0" en éxito, -1 en error.
 * ***************************************************************************/
int Signal::wait_and_handle (int signal)
{
    sigset_t mask;

    if (sigfillset(&mask) != 0)
    {
        perror( ERROR("Couldn't start waiting for signal with sigfillset.\n"));
        return -1;
    }

    if (sigdelset(&mask, signal) != 0)
    {
        perror( ERROR("Couldn't start waiting for signal with sigdelset.\n"));
        return -1;
    }

    sigsuspend(&mask); // Always returns -1 for signal interruption.

    return 0;

}

/******************************************************************************
 *  @brief:     Función bloqueante hasta que se reciba la "signal", momento en
 *              el que ignora la señal y continua con la ejecución normal del
 *              programa.
 * 
 *  @arg:       signal: señal a esperar.
 * 
 *  @return:    "0" en éxito, -1 en error.
 * ***************************************************************************/
int Signal::wait_and_continue (int signal)
{
    sigset_t mask;
    int sig_return = 0;

    if (sigemptyset(&mask) != 0)
    {
        perror( ERROR("Couldn't start waiting for signal with sigemptyset.\n"));
        return -1;
    }

    if (sigaddset(&mask, signal) != 0)
    {
        perror( ERROR("Couldn't start waiting for signal with sigaddset.\n"));
        return -1;
    }

    if (sigwait(&mask, &sig_return) != 0)
    {
        perror( ERROR("Something happened while waiting for the signal with sigwait.\n"));
        return -1;
    }

    return 0;
}

