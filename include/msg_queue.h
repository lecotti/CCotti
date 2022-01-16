#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/msg.h>
#include "tools.h"

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
    MsgQueue(const char* path, int id);
    int send(msg_t msg, int mtype=1);
    msg_t rcv(int mtype=0);
    MsgQueue& operator<<(msg_t msg);
    MsgQueue& operator>>(msg_t& msg);
    void free();
};

/******************************************************************************
 *  @brief:     Instancia una cola de mensajes.
 * 
 *  @arg:       <msg_t>: el tipo del mensaje. Puede ser cualquier cosa, incluso una
 *              struct autodefinida. En cualquier caso, su tamaño debe estar per-
 *              fectamente definido. Por ejemplo, un vector de chars se define como
 *              char[N], de tamaño N, y no un char *.
 * 
 *              path: Identificador inequívoco de la queue. Debe ser un path válido. 
 * 
 *              id: Identificador inequívoco de la queue.
 * 
 *  @return:    None.
 * ***************************************************************************/
template <class msg_t>
MsgQueue<msg_t>::MsgQueue(const char* path, int id)
{
    key_t key;

    if ( (key = ftok(path, id) ) == -1)
    {
        perror(RED("ftok.\n"));
        return;
    }

    if ( (this->msg_id = msgget(key, IPC_CREAT | 0666) ) == -1)
    {
        perror(RED("msgget.\n"));
        return;
    }
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
int MsgQueue<msg_t>::send(msg_t msg, int mtype)
{
    struct msgbuf sending_msg;

    if (mtype <= 0)
    {
        mtype = 1;
    }

    sending_msg.mtype = (long) mtype;
    sending_msg.msg = msg;

    return msgsnd(this->msg_id, &sending_msg, (size_t) sizeof(msg_t), 0);
}


/******************************************************************************
 *  @brief:     Recibe un mensaje.
 * 
 *  @arg:       value: El valor que tomará el semáforo. 
 * 
 *              semnum: El número de semáforo.
 * 
 *  @return:    "0" en éxtio, "-1" en error.
 * ***************************************************************************/
template <class msg_t>
msg_t MsgQueue<msg_t>::rcv(int mtype)
{
    struct msgbuf output;

    if( msgrcv(this->msg_id, &output, (size_t) sizeof(msg_t), (long) mtype, 0) == -1)
    {
        perror(RED("msgrcv.\n"));
    }

    return output.msg;
}

/******************************************************************************
 *  @brief:     Envia un mensaje a la cola.
 * 
 *  @arg:       msg: El mensaje a enviar.
 * 
 *  @return:    Una referencia al objeto.
 * ***************************************************************************/
template <class msg_t>
MsgQueue<msg_t>& MsgQueue<msg_t>::operator<<(msg_t msg)
{
    this->send(msg, 1);
    return *this;
}

/******************************************************************************
 *  @brief:     Recibe el primer mensaje de la cola.
 * 
 *  @arg:       msg: Una referencia al lugar donde se guardará el mensaje.
 * 
 *  @return:    Una referencia al objeto.
 * ***************************************************************************/
template <class msg_t>
MsgQueue<msg_t>& MsgQueue<msg_t>::operator>>(msg_t& msg)
{
    msg = this->rcv(0);
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