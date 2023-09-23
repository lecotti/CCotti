#include "sem.h"

/// @brief Creates a semaphore with an initial "semval" value of 1,
///  or connects to an existing one. See "Sem::op" to know how to operate it.
/// @param path Path to any file. Necessary to identify the semaphore.
/// @param id Can be any value. Identifies the semaphore.
/// @param create If "true", the semaphore will be created. If "false", it will
///  try to connect to an already existing one, with the same "path" and "id".
/// @return Throws "std::runtime_error" in case of error.
Sem::Sem(const char* path, int id, bool create): creator(create) {
    key_t key;
    this->pid = gettid();
    if ( (key = ftok(path, id) ) == -1) {
        perror(ERROR("ftok in Sem::Sem"));
        throw(std::runtime_error("ftok"));
    }
    if (create) {
        if ( (this->semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666) ) == -1 ) {
            perror(ERROR("semget in Sem::Sem"));
            throw(std::runtime_error("semget"));
        }
        if (this->set(1) != 0) {
            throw(std::runtime_error("set"));
        }
    } else {
        if ( (this->semid = semget(key, 0, 0) ) == -1 ) {
            perror(ERROR("semget in Sem::Sem"));
            throw(std::runtime_error("semget"));
        }
    }
}

/// @brief  Destroys the semaphore, free resources. Only the creator will be
///  be able to remove it.
Sem::~Sem(void) {
    if (this->creator && this->pid == gettid()) {
        if (semctl(this->semid, 0, IPC_RMID) == -1) {
            perror(ERROR("semctl in Sem::~Sem"));
        }
    }
}

/// @brief Checks if the semaphore already exists.
/// @param path Path to any file. Necessary to identify the semaphore.
/// @param id Can be any value. Identifies the semaphore.
/// @return "true" if it already exists, "false" otherwise.
bool Sem::exists(const char* path, int id) {
    key_t key;
    if ( (key = ftok(path, id) ) == -1) {
        return false;
    }
    if ( (semget(key, 0, 0) ) == -1 ) {
        return false;
    }
    return true;
}

/// @brief Sets the semaphore's "semval" to a specific value.
/// @return "0" on success, "-1" on error.
int Sem::set (unsigned int value) {
    if (semctl(this->semid, 0, SETVAL, (int) value) == -1) {
        perror(ERROR("semctl in Sem::set"));
        return -1;
    }
    return 0;
}

/// @brief Returns the value of the semaphore, or "-1" on error.
int Sem::get(void) const {
    int sem_val;
    if ((sem_val = semctl(this->semid, 0, GETVAL)) == -1) {
        perror(ERROR("semctl in Sem::get"));
    }
    return sem_val;
}

/// @brief Realizes one of the following operations, depending on the argument "op":
///  op = 0;  The proccess blocks until the value of the semaphore equals "0".
///  op > 0;  Adds that value to the semaphore "semval".
///  op < 0;  If |op| <= |semval|, then semval = semval - op
///           If |op| >  |semval|, then the operation blocks until "op" can be
///           subtracted from "semval", with the end result a positive number or "0".
/// @param op Operation value.
/// @return "0" on success, "-1" on error.
int Sem::op (int op) {
    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = op;
    sop.sem_flg = 0;
    if (semop(this->semid, &sop, 1) == -1) {
        perror(ERROR("semop in Sem::op"));
        return -1;
    }
    return 0;
}

/******************************************************************************
 * Overloaded operators
******************************************************************************/

/// @brief Adds "+1" to the semaphore's "semval".
int Sem::operator++ (int) {
    this->op(1);
    return this->get();
}

int Sem::operator++ () {
    this->op(1);
    return this->get();
}

/// @brief Subtracts "-1" to the semaphore's "semval".
int Sem::operator-- (int) {
    this->op(-1);
    return this->get();
}

int Sem::operator-- () {
    this->op(-1);
    return this->get();
}

/// @brief Sets the semaphore's value
int Sem::operator= (int a) {
    this->set(a);
    return this->get();
}

/// @brief Adds and arbitrary amount to the semaphore. Doesn't update the value.
int Sem::operator+ (int a) {
    return this->get() + a;
}

/// @brief Subtracts and arbitrary amount to the semaphore. Doesn't update the value.
int Sem::operator- (int a) {
    return this->get() - a;
}
