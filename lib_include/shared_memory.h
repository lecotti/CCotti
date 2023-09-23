#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "tools.h"
#include <stdexcept>
#include <unistd.h>

/******************************************************************************
 * Class definition
******************************************************************************/

template <class data_t>
class SharedMemory {
private:
    int shmid;
    data_t* shmaddr;
    pid_t pid;
    bool creator;

public:
    SharedMemory(const char* path, int id, size_t size=0);
    ~SharedMemory();

    void write( data_t* elements, int size, int index=0);
    void write(data_t element, int index);
    void read(data_t* array, int size, int index=0);
    data_t read(int index);
    static bool exists(const char* path, int id);

    void operator= (data_t element);
    SharedMemory<data_t>& operator<< (data_t element);
    data_t& operator[] (int index);
};

/******************************************************************************
 * Template functions definition
******************************************************************************/

/// @brief Creates or connects to a shared memory.
/// @tparam data_t The type of data stored in the shared memory. Its size must
///  fixed and known at compile time. Don't use fixed length arrays like char[20],
///  instead use structs like struct data {char[20]}.
/// @param path Any file path. Identifies the shm.
/// @param id Any number. Identifies the shm.
/// @param size If size > 0, then the shared memory is created with the size to
///  allocate "size" elements of type "data_t".
///  already existing one (default = 0).
/// @return On error, std::runtime_error() is thrown.
template <class data_t>
SharedMemory<data_t>::SharedMemory(const char* path, int id, size_t size) {
    key_t key;
    this->pid = gettid();
    if ( (key = ftok(path, id) ) == -1) {
        perror( ERROR("ftok in SharedMemory::SharedMemory"));
        throw(std::runtime_error("ftok"));
    }
    if (size) {  // Create new
        this->creator = true;
        if( (this->shmid = shmget(key, (size_t)size*sizeof(data_t), IPC_CREAT | IPC_EXCL | 0666) ) == -1) {
            perror(ERROR("shmget in SharedMemory::SharedMemory"));
            throw(std::runtime_error("shmget"));
        }
    } else { // Connect to existing one
        this->creator = false;
        if( (this->shmid = shmget(key, 0, 0) ) == -1) {
            perror(ERROR("shmget in SharedMemory::SharedMemory"));
            throw(std::runtime_error("shmget"));
        }
    }
    if ( (this->shmaddr = (data_t*) shmat(this->shmid, NULL, 0)) == (data_t*) -1) {
        perror(ERROR("shmat in SharedMemory::SharedMemory"));
        throw(std::runtime_error("shmat"));
    }
}

/// @brief Detaches pointer from shm. If you are the creator, destroy the shm.
template <class data_t>
SharedMemory<data_t>::~SharedMemory() {
    if (shmdt((void *) this->shmaddr) == -1) {
        perror(ERROR("shmdt in SharedMemory::~SharedMemory"));
    }
    if (this->creator && this->pid == gettid()) {
        if (shmctl(this->shmid, IPC_RMID, NULL) == -1) {
            perror(ERROR("shmctl in SharedMemory::~SharedMemory"));
        }
    }
}

/// @brief Writes multiple elements to the shared memory
/// @param elements Vector with the elements to be written.
/// @param size Size of the vector.
/// @param index Position from where to start writing in the shared memory.
template <class data_t>
void SharedMemory<data_t>::write(data_t* elements, int size, int index) {
    for (int i = 0; i < size; i++) {
        this->shmaddr[index + i] = elements[i];
    }
}

/// @brief Writes a single element to the shared memory.
/// @param element Element to be written.
/// @param index Position in the shared memory.
template <class data_t>
void SharedMemory<data_t>::write(data_t element, int index) {
    this->shmaddr[index] = element;
}

/// @brief Returns a copy of the elements in the shared memory.
/// @param array Place where the elements will be copied.
/// @param size Size of the array.
/// @param index Place from where to start reading the shared memory.
template <class data_t>
void SharedMemory<data_t>::read(data_t* array, int size, int index) {
    for (int i = 0; i < size; i++) {
        array[i] = this->shmaddr[index + i];
    }
}

/// @brief Returns a single copy of an element from the shared memory.
/// @param index Position in the shared memory.
/// @return The copy of the value.
template <class data_t>
data_t SharedMemory<data_t>::read(int index) {
    return this->shmaddr[index];
}

/// @brief Checks if the shared memory exists.
/// @param path Any file path. Identifies the shm.
/// @param id Any number. Identifies the shm.
/// @return "true" if it exists, "false" otherwise.
template <class data_t>
bool SharedMemory<data_t>::exists(const char* path, int id) {
    key_t key;
    if ( ( key = ftok(path, id) ) == -1) {
        return false;
    }
    if (shmget(key, 0, 0) == -1) {
        return false;
    }
    return true;
}

/******************************************************************************
 * Overloaded operators
******************************************************************************/

/// @brief Write on first position.
template <class data_t>
void SharedMemory<data_t>::operator= (data_t element) {
    this->write(element, 0);
}

/// @brief Write on first position.
template <class data_t>
SharedMemory<data_t>& SharedMemory<data_t>::operator<< (data_t element) {
    this->write(element, 0);
    return *this;
}

/// @brief Return a modifiable reference to the value at "index".
template <class data_t>
data_t& SharedMemory<data_t>::operator[] (int index) {
    return this->shmaddr[index];
}

#endif //SHARED_MEM_H
