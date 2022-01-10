template <class msg_t>
MsgQueue<msg_t>::MsgQueue(const char* path, int id)
{
    key_t key;

    if ( (key = ftok(path, id) ) == -1)
    {
        perror(RED("ftok.\n"));
        return
    }

    if ( (this->msg_id = msgget(key, IPC_CREAT | 0666) ) == -1)
    {
        perror(RED("msgget.\n"));
        return
    }
}

template <class msg_t>
int MsgQueue<msg_t>::send(arg_t msg)
{
    return msgsnd(this->msg_id, msg_t )
}

