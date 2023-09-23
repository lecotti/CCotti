#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/msg.h>
#include "tools.h"
#include <stdexcept>
#include <errno.h>
#include <unistd.h>

template <class msg_t>
class MsgQueue {
private:
    struct msgbuf {
        long mtype;
        msg_t msg;
    };
    int msg_id;
    bool creator;
    pid_t pid;

public:
    MsgQueue(const char* path, int id, bool create=false);
    ~MsgQueue();
    bool static exists(const char* path, int id);
    int write(msg_t msg, long mtype=1);
    msg_t read(int mtype=0, int* status=NULL, int flags=0);
    msg_t peek(int index, int* status=NULL);
    int get_msg_qtty(void);
    bool is_empty(void);
    bool has_msg(void);
    MsgQueue& operator<<(msg_t msg);
    MsgQueue& operator>>(msg_t& msg);
};

/******************************************************************************
 * Template functions
******************************************************************************/

/// @brief Creates a message queue.
/// @tparam msg_t Type of the message to be passed along. Its size must be fixed
///   and defined at compile time. Don't use fixed length arrays (char[20]),
///   instead use structs like struct msg {char[20]}.
/// @param path Can be any path. Identifier for the message queue.
/// @param id Can be any number. Identifier for the message queue.
/// @param create If "true", create the queue. If "false", connect to an
///  already existing one.
/// @return If error, throws an exception with std::runtime_error
template <class msg_t>
MsgQueue<msg_t>::MsgQueue(const char* path, int id, bool create): creator(create) {
    key_t key;
    int flags = (create) ? (IPC_CREAT | 0666) : 0;
    this->pid = gettid();
    if ( (key = ftok(path, id) ) == -1) {
        perror(ERROR("ftok in MsgQueue::MsgQueue"));
        throw(std::runtime_error("ftok"));
    }
    if (create) {
        if ( (this->msg_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666) ) == -1) {
            perror(ERROR("msgget in MsgQueue::MsgQueue"));
            throw(std::runtime_error("msgget"));
        }
    } else {
        if ( (this->msg_id = msgget(key, 0)) == -1 ) {
            perror(ERROR("msgget in MsgQueue::MsgQueue"));
            throw(std::runtime_error("msgget"));
        }
    }
}

/// @brief Destroy the queue only if you are the creator, and are in the same
///  thread or proccess.
template <class msg_t>
MsgQueue<msg_t>::~MsgQueue(void) {
    if (this->creator && this->pid == gettid()) {
        if (msgctl(this->msg_id, IPC_RMID, NULL) == -1) {
            perror(ERROR("msgctl in MsgQueue::~MsgQueue"));
        }
    }
}

/// @brief Checks if the message queue already exists.
/// @return "true" if it exists, "false" otherwise.
template <class msg_t>
bool MsgQueue<msg_t>::exists(const char* path, int id) {
    key_t key;
    if ( (key = ftok(path, id) ) == -1) {
        return false;
    }
    if (msgget(key, 0) == -1) {
        return false;
    }
    return true;
}

/// @brief Writes a message in the queue.
/// @param msg Message to be written.
/// @param mtype Message identifier (default "1").
/// @return "0" on success, "-1" on error.
template <class msg_t>
int MsgQueue<msg_t>::write(msg_t msg, long mtype) {
    struct msgbuf sending_msg;
    if (mtype <= 0) {
        mtype = 1;
    }
    sending_msg.mtype = mtype;
    sending_msg.msg = msg;
    if (msgsnd(this->msg_id, &sending_msg, (size_t) sizeof(msg_t), 0) == -1) {
        perror(ERROR("msgsnd in MsgQueue::write"));
        return -1;
    }
    return 0;
}

/// @brief Reads the queue. By default, in a blocking manner.
/// @param mtype Dictates which message to get from the queue:
///  * mtype = 0; Reads first message (FIFO).
///  * mtype > 0; Reads the first message written with the same mtype.
///  * mtype < 0; Reads the first message with the lowest mtype, if its absolute
///  value is equal lower. For example, if mtype = -2; You can read messages with
///  mtype = 1 or mtype = 2, in that order.
/// @param status If not NULL, it will be loaded with a "0" on success, or with
///  the "errno" value in case of error.
/// @param flags It can be a logical "or" of the following options:
///  * IPC_NOWAIT; not blocking read.
///  * MSG_COPY | IPC_NOWAIT; Copies the message from the queue in position
///  "mtype". The message is not removed, and if no message is present in that
///  position fails immediately.
/// @return The value returned from the message queue.
template <class msg_t>
msg_t MsgQueue<msg_t>::read(int mtype, int* status, int flags) {
    struct msgbuf output;
    int error_state = 0;
    if( msgrcv(this->msg_id, &output, (size_t) sizeof(msg_t), (long) mtype, flags) == -1) {
        error_state = errno;
        perror(ERROR("msgrcv in MsgQueue::read"));
    }
    if (status != NULL) {
        *status = error_state;
    }
    return output.msg;
}

/// @brief Returns a copy of a message in the queue, without popping it.
/// @param index Position in the queue, starting with "0".
/// @param status If "0", the value was retrieved successfully. If != 0, then
///  no message was found, and the value returned is junk.
template <class msg_t>
msg_t MsgQueue<msg_t>::peek(int index, int* status) {
    struct msgbuf output;
    output.msg = this->read(index, status, IPC_NOWAIT | MSG_COPY);
    return output.msg;
}

/// @brief Returns the amount of messages in the queue, or "-1" on error.
template <class msg_t>
int MsgQueue<msg_t>::get_msg_qtty(void) {
    struct msqid_ds info;
    if (msgctl(this->msg_id, MSG_STAT, &info) == -1) {
        perror(ERROR("msgctl in MsgQueue::get_msg_qtty"));
        return -1;
    }
    return (int) info.msg_qnum;
}

/// @brief Returns "true" if the queue is empty, "false" otherwise.
template <class msg_t>
bool MsgQueue<msg_t>::is_empty(void) {
    return (this->get_msg_qtty() == 0);
}

/// @brief Returns "true" if there is at least one message in the queue,
///  "false" otherwise.
template <class msg_t>
bool MsgQueue<msg_t>::has_msg(void) {
    return (this->get_msg_qtty() > 0);
}

/******************************************************************************
 * Overloaded operators
******************************************************************************/

/// @brief Sends a message with "mtype=1". Might throw "std::runtime_error".
template <class msg_t>
MsgQueue<msg_t>& MsgQueue<msg_t>::operator<<(msg_t msg) {
    if (this->write(msg, 1) == -1) {
        throw(std::runtime_error("MsgQueue::operator<<"));
    }
    return *this;
}

/// @brief Reads the first message on the queue. Might throw "std::runtime_error".
template <class msg_t>
MsgQueue<msg_t>& MsgQueue<msg_t>::operator>>(msg_t& msg) {
    int status;
    msg = this->read(0, &status);
    if (status != 0) {
        throw(std::runtime_error("MsgQueue::operator>>"));
    }
    return *this;
}

#endif // MSQUEUE_H
