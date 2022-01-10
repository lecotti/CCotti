#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/msg.h>
#include "../headers/tools.h"

template <class msg_t>
class MsgQ
{
private:
    struct msgbuf 
    {
        long mtype;
        msg_t mtext; 
    };

    int msg_id;

public:
};