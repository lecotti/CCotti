#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/msg.h>
#include "tools.h"
#include <stdexcept>
#include <errno.h>

template <class msg_t>
class MsgQueue
{
private:
    struct msgbuf 
    {
        long mtype;
        msg_t msg; 
    };

    int msg_id;

public:
    MsgQueue(const char* path, int id, bool create=false);
    bool static exists(const char* path, int id);
    int write(msg_t msg, int mtype=1);
    msg_t read(int mtype=0, int* status=NULL, int flags=0);
    msg_t peek(int index, int* status=NULL);
    int get_msg_qtty(void);
    bool is_empty(void);
    bool has_msg(void);
    MsgQueue& operator<<(msg_t msg);
    MsgQueue& operator>>(msg_t& msg);
    void free();
};

/******************************************************************************
 *  @brief:     Instancia una cola de mensajes.
 * 
 *  @arg:       <msg_t>: el tipo del mensaje. Su tamaño debe estar perfectamente
 *              definido, pero no puede ser un puntero del tipo char[20], pero sí
 *              una struct con un char[20] dentro.
 * 
 *              path: Identificador inequívoco de la queue. Debe ser un path válido. 
 * 
 *              id: Identificador inequívoco de la queue.
 * 
 *              create: si la msgq debe ser creada o no.
 * 
 *  @return:    If error, throws an exception with std::runtime_error
 * ***************************************************************************/
template <class msg_t>
MsgQueue<msg_t>::MsgQueue(const char* path, int id, bool create)
{
    key_t key;
    int flags = (create) ? (IPC_CREAT | 0666) : 0;

    if ( (key = ftok(path, id) ) == -1)
    {
        perror(ERROR("Couldn't create | get msgq with ftok.\n"));
        throw(std::runtime_error("ftok"));
    }

    if (create)
    {
        if ( (this->msg_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666) ) == -1)
        {
            perror(ERROR("Couldn't create msgq with msgget.\n"));
            throw(std::runtime_error("msgget"));
        }
    }

    else
    {
        if ( (this->msg_id = msgget(key, 0)) == -1 )
        {
            perror(ERROR("Couldn't connect to already existing msgq with msgget.\n"));
            throw(std::runtime_error("msgget"));
        }
    }
    
}

/******************************************************************************
 *  @brief:     Chequea si la cola de mensajes existe.
 * 
 *  @arg:       path and id: identificadores.
 * 
 *  @return:    "true" en éxito, "false" si no existe o hubo un error.
 * ***************************************************************************/
template <class msg_t>
bool MsgQueue<msg_t>::exists(const char* path, int id)
{
    key_t key;

    if ( (key = ftok(path, id) ) == -1)
    {
        return false;
    }

    if (msgget(key, 0) == -1)
    {
        return false;
    }

    return true;
}


/******************************************************************************
 *  @brief:     Envia el mensaje a la cola.
 * 
 *  @arg:       msg: El mensaje a enviar. 
 * 
 *              mtype: Identificador del mensaje. Debe ser un número positivo.
 * 
 *  @return:    "0" en éxtio, "-1" en error.
 * ***************************************************************************/
template <class msg_t>
int MsgQueue<msg_t>::write(msg_t msg, int mtype)
{
    struct msgbuf sending_msg;

    if (mtype <= 0)
    {
        mtype = 1;
    }

    sending_msg.mtype = (long) mtype;
    sending_msg.msg = msg;

    if (msgsnd(this->msg_id, &sending_msg, (size_t) sizeof(msg_t), 0) == -1)
    {
        perror(ERROR("Couldn't send msg with msgsnd.\n"));
        return -1;
    }

    return 0;
}


/******************************************************************************
 *  @brief:     Recibe un mensaje de manera bloqueante y lo saca de la cola.
 * 
 *  @arg:       mtype: depende el valor, será el orden en que agarra los mensajes:
 *              mtype = 0 -> Lee el primer mensaje (FIFO).
 *              mtype > 0 -> Lee el primer mensaje con exactamente mtype.
 *              mtype < 0 -> Lee el primer mensaje con mtype lo más chico posible, siempre
 *              que sea menor al valor suministrado.
 * 
 *              flags: puede ser un "or" lógico de los siguientes modos:
 *              0 -> Bloqueante hasta que haya un mensaje (por defecto).
 *              IPC_NOWAIT -> No bloqueante.
 *              MSG_COPY | IPC_NOWAIT (mtype = index) -> Saca un mensaje de manera
 *                  no destructivade la cola, siendo mtype la posición en la que 
 *                  esté el mensaje.
 * 
 *              status: Si no es NULL, rellena la variable con "0" en caso de éxito,
 *              con "-1" si se especifico IPC_NOWAIT y no se encontró el mensaje, 
 *              o el valor de "errno" en caso de error (> 0). 
 * 
 *  @return:    El valor retornado de la cola de mensajes. Si status != 0, el
 *              valor retornado es basura.
 * ***************************************************************************/
template <class msg_t>
msg_t MsgQueue<msg_t>::read(int mtype, int* status, int flags)
{
    struct msgbuf output;
    int state = 0;

    if( msgrcv(this->msg_id, &output, (size_t) sizeof(msg_t), (long) mtype, flags) == -1)
    {
        state = errno;
        if (state == ENOMSG) // se especifico IPC_NOWAIT y no se encontró mensaje.
        {
            state = -1;
        }

        else
        {
            perror(ERROR("Couldn't get a message from the queue with msgrcv.\n"));
        }
    }

    if (status != NULL)
    {
        *status = state;
    }

    return output.msg;
}

/******************************************************************************
 *  @brief:     Revisa que exista un mensaje en la cola en la posición "index",
 *              de manera no bloqueante, y devuelve una copia, sin eliminarlo de
 *              la cola en sí.
 * 
 *  @arg:       index: posición del mensaje en la cola (empezando en 0).
 * 
 *              status: rellena la variable con "0" en éxito, con "-1" si el mensaje
 *              no existe todavía, o con el valor de "errno" en caso de error (> 0).
 * 
 *  @return:    Una COPIA del valor de la cola, sin haberlo eliminado de la misma.
 *              Si status != 0, el valor de retorno es basura.
 * ***************************************************************************/
template <class msg_t>
msg_t MsgQueue<msg_t>::peek(int index, int* status)
{
    struct msgbuf output;
    output.msg = this->read(index, status, IPC_NOWAIT | MSG_COPY);

    return output.msg;
}

/******************************************************************************
 *  @brief:     Indica cuántos mensajes hay en la cola.
 * 
 *  @arg:       Void.
 * 
 *  @return:    La cantidad de mensajes, o "-1" en error.
 * ***************************************************************************/
template <class msg_t>
int MsgQueue<msg_t>::get_msg_qtty(void)
{
    struct msqid_ds info;
    if (msgctl(this->msg_id, MSG_STAT, &info) == -1)
    {
        perror(ERROR("Couldn't check the aomunt of msgs in queue with msgctl.\n"));
        return -1;
    }

    return (int) info.msg_qnum;
}

/******************************************************************************
 *  @brief:     Indican si la queue está vacía o si tiene algún mensaje.
 * 
 *  @arg:       Void.
 * 
 *  @return:    "true" si está vacía | con algún mensjae, "false" en caso contrario.
 * ***************************************************************************/
template <class msg_t>
bool MsgQueue<msg_t>::is_empty(void)
{
    return (this->get_msg_qtty() == 0);
}

template <class msg_t>
bool MsgQueue<msg_t>::has_msg(void)
{
    return (this->get_msg_qtty() > 0); //Puede ser -1 en error
}

/******************************************************************************
 *  @brief:     Envia un mensaje a la cola con mtype = 1.
 * 
 *  @arg:       msg: El mensaje a enviar.
 * 
 *  @return:    Una referencia al objeto.
 * ***************************************************************************/
template <class msg_t>
MsgQueue<msg_t>& MsgQueue<msg_t>::operator<<(msg_t msg)
{
    this->write(msg, 1);
    return *this;
}

/******************************************************************************
 *  @brief:     Recibe el primer mensaje de la cola. No chequea errores.
 * 
 *  @arg:       msg: Una referencia al lugar donde se guardará el mensaje.
 * 
 *  @return:    Una referencia al objeto.
 * ***************************************************************************/
template <class msg_t>
MsgQueue<msg_t>& MsgQueue<msg_t>::operator>>(msg_t& msg)
{
    msg = this->read();
    return *this;
}

/******************************************************************************
 *  @brief:     Destruye la cola de mensajes.
 * 
 *  @arg:       Void.
 * 
 *  @return:    Void.
 * ***************************************************************************/
template <class msg_t>
void MsgQueue<msg_t>::free()
{
    msgctl(this->msg_id, IPC_RMID, NULL);
}


#endif // MSQUEUE_H