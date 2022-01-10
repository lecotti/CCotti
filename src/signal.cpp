#include "../headers/signal.h"

/******************************************************************************
 *  STATIC VARIABLES
 * ***************************************************************************/
bool        Signal::mask_initialized = false;
sigset_t    Signal::sig_mask;


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
 *  @return:    None.
 * ***************************************************************************/
Signal::Signal (int signal, void(*signal_handler)(int), int flags): signal(signal)
{
    if (!Signal::mask_initialized)
    {
        Signal::unblock_all();
    }

    this->sa.sa_mask = this->sig_mask;
    
    this->sa.sa_handler = signal_handler;
    
    this->sa.sa_flags = flags;
    
    if (sigaction (signal, &(this->sa), NULL) == -1)
    {
        perror (RED("Sigaction.\n"));
        return;
    }
}

/******************************************************************************
 *  @brief:     Cambia el signal handler.
 * 
 *  @arg:       signal_handler: el nuevo signal_handler.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::change_handler(void(*signal_handler)(int))
{
    this->sa.sa_handler = signal_handler;
    return this->update();
}

/******************************************************************************
 *  @brief:     Ignora la señal, de modo que si es detectada no será procesada
 *              (SIG_IGN).
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::ignore_signal(void)
{
    return this->change_handler(SIG_IGN);
}

/******************************************************************************
 *  @brief:     Le otorga a la señal su handler por defecto (SIG_DFL).
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::restore_default_handler(void)
{
    return this->change_handler(SIG_DFL);
}

/******************************************************************************
 *  @brief:     Bloquea una señal. Esto significa que esta no será relevada por
 *              este thread.
 * 
 *  @arg:       signal: La señal a bloquear.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::block_signal(int signal)
{
    if (!Signal::mask_initialized)
    {
        if (Signal::unblock_all() == -1)
        {
            return -1;
        }
    }

    if ( sigaddset( &(Signal::sig_mask), signal) == 0 )
    {
        return pthread_sigmask(SIG_BLOCK, &(Signal::sig_mask), NULL);
    }

    return -1;
}

/******************************************************************************
 *  @brief:     Desbloquea una señal previamente bloqueada.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::unblock_signal(int signal)
{
    sigset_t mask;

    if (!Signal::mask_initialized)
    {
        if (Signal::unblock_all() == -1)
        {
            return -1;
        }
    }

    if (sigemptyset(&mask) == 0)
    {
        if ( sigaddset( &(Signal::sig_mask), signal) == 0 )
        {
            return pthread_sigmask(SIG_UNBLOCK, &(Signal::sig_mask), NULL);
        }
    }

    return -1;

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
    if (sigemptyset ( &(Signal::sig_mask) ) == -1)
    {
        return -1;
    }

    Signal::mask_initialized = true;

    return pthread_sigmask(SIG_SETMASK, &(Signal::sig_mask), NULL);
}

/******************************************************************************
 *  @brief:     Envia una señal al PID.
 * 
 *  @arg:       pid: Process ID.
 *              signal: Señal a mandar.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::kill (pid_t pid, int signal)
{
    return kill(pid, signal);
}

/******************************************************************************
 *  @brief:     Función bloqueante hasta que se reciba la "signal", momento en
 *              el que ejecuta el handler que tenga asociado.
 * 
 *  @arg:       signal: señal a esperar.
 * 
 *  @return:    Void.
 * ***************************************************************************/
void Signal::pause (int signal)
{
    sigset_t mask;

    sigfillset(&mask);
    sigdelset(&mask, signal);

    sigsuspend(&mask);
}

/******************************************************************************
 *  @brief:     Actualiza el comportamiento de la señal.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "0" en éxito, "-1" en error.
 * ***************************************************************************/
int Signal::update(void)
{
    return sigaction(this->signal, &(this->sa), NULL);
}