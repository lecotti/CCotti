#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "tools.h"
#include <stdexcept>

template <class arg_t>
class SharedMemory
{
private:
    int shmid;
    arg_t* shmaddr;

public:

    SharedMemory(const char* path, int id, int size=0);

    void write( arg_t* elements, int size, int index=0);
    void write(arg_t element, int index);
    
    arg_t* read(arg_t* array, int size, int index=0);
    arg_t read(int index);

    static bool exists(const char* path, int id);
    int free(void);
    ~SharedMemory();

    void operator= (arg_t element);
    SharedMemory<arg_t>& operator<< (arg_t element);
    arg_t& operator[] (int index);
};


/******************************************************************************
 *  @brief:     Crea o instancia una shared memory: un espacio de memoria compartible
 *              entre procesos. Se representa como un puntero del tipo <arg_t>, de
 *              tamaño size. No puede ser un puntero del tipo char[20], pero si una
 *              struct con un char[20] dentro.
 *              **IMPORTANTE** Liberar recursos con free().
 * 
 *  @arg:       <arg_t>: El tipo de dato, puede ser cualquiera.
 * 
 *              size: El tamaño del vector. Si es "0", no crea una nueva, sino que
 *              instancia una ya creada por otro proceso.
 * 
 *              path: path a un archivo, forma de identificar inequívocamente a una
 *              shared memory.
 * 
 *              id: forma de identificar inequívocamente a una shared memory.
 * 
 *  @return:    Levanta una excepción con el valor de std::runtime_error si error.
 * ***************************************************************************/
template <class arg_t>
SharedMemory<arg_t>::SharedMemory(const char* path, int id, int size)
{
    key_t key;
    if ( (key = ftok(path, id) ) == -1)
    {
        perror( ERROR("Couldn't create shared memory with ftok.\n"));
        throw(std::runtime_error("ftok"));
    }

    if (size)   // Create new
    {
        if( (this->shmid = shmget(key, (size_t)size*sizeof(arg_t), IPC_CREAT | IPC_EXCL | 0666) ) == -1)
        {
            perror(ERROR("Couldn't create shared memory with shmget.\n"));
            throw(std::runtime_error("shmget"));
        }
    }

    else    // Connect to existing one
    {
        if( (this->shmid = shmget(key, 0, 0) ) == -1)
        {
            perror(ERROR("Shmget: shared_memory doesn't exist.\n"));
            throw(std::runtime_error("shmget"));
        }
    }
    

    if ( (this->shmaddr = (arg_t*) shmat(this->shmid, NULL, 0)) == (arg_t*) -1)
    {
        perror(ERROR("Couldn't attach pointer to shared memory with shmaddr.\n"));
        throw(std::runtime_error("shmat"));
    }

}

/******************************************************************************
 *  @brief:     Escribe en la shared memory muchos elementos a la vez.
 * 
 *  @arg:       elements: vector con los elementos a escribir.
 * 
 *              size: El tamaño del vector.
 * 
 *              index: La posición desde la cual empezar a escribir.
 * 
 *  @return:    Void.
 * ***************************************************************************/
template <class arg_t>
void SharedMemory<arg_t>::write( arg_t* elements, int size, int index)
{
    for (int i = 0; i < size; i++)
    {
        this->shmaddr[index + i] = elements[i];
    }
}

/******************************************************************************
 *  @brief:     Escribe en la shared memory un único elemento.
 * 
 *  @arg:       element: elemento a escribir.
 * 
 *              index: La posición en la cual escribirlo.
 * 
 *  @return:    Void.
 * ***************************************************************************/
template <class arg_t>
void SharedMemory<arg_t>::write(arg_t element, int index)
{
    this->shmaddr[index] = element;
}

/******************************************************************************
 *  @brief:     Lee varios elementos de la shared memory y devuelve UNA COPIA.
 * 
 *  @arg:       array: Lugar donde se guardarán los elementos.
 * 
 *              size: El tamaño del array.
 * 
 *              index: El lugar desde dónde empezar a leer.
 * 
 *  @return:    El puntero "array" pasado como argumento con COPIAS de los elementos.
 * ***************************************************************************/
template <class arg_t>
arg_t* SharedMemory<arg_t>::read(arg_t* array, int size, int index)
{
    for (int i = 0; i < size; i++)
    {
        array[i] = this->shmaddr[index + i];
    }
    return array;
}

/******************************************************************************
 *  @brief:     Lee un elemento de la shared memory, y devuelve UNA COPIA.
 * 
 *  @arg:       index: El lugar a leer.
 * 
 *  @return:    UNA COPIA del elemento leido.
 * ***************************************************************************/
template <class arg_t>
arg_t SharedMemory<arg_t>::read(int index)
{
    return this->shmaddr[index];
}

/******************************************************************************
 *  @brief:     Checks whether the shared memory is created or not.
 * 
 *  @arg:       path and id: identify the shared memory.
 * 
 *  @return:    "true" if exists, "false" otherwise.
 * ***************************************************************************/
template <class arg_t>
bool SharedMemory<arg_t>::exists(const char* path, int id)
{
    key_t key;
    if ( ( key = ftok(path, id) ) == -1)
    {
        return false;
    }

    if (shmget(key, 0, 0) == -1)
    {
        return false;
    }

    return true;    // Existance
}


/******************************************************************************
 *  @brief:     Elimina la shared memory. Esta función debe ser llamada de forma
 *              manual porque si se creasen hijos con "fork()", entonces los hijos
 *              heredarían copias del objeto, y al terminar la ejecución de un hijo
 *              destruiría la shared memory.
 * 
 *  @arg:       Void. 
 * 
 *  @return:    "0" en éxito, -1 en error.
 * ***************************************************************************/
template <class arg_t>
int SharedMemory<arg_t>::free(void)
{
    if (shmdt((void *) this->shmaddr) != 0)
    {
        perror( ERROR("Couldn't detach from shared memory with shmdt.\n"));
        //Don't return.
    }
    if (shmctl(this->shmid, IPC_RMID, NULL) != 0)
    {
        perror( ERROR("Couldn't destroy shared memory with shmctl.\n"));
        return -1;
    }

    return 0;
}

/******************************************************************************
 *  @brief:     Destructor. Desacopla el puntero a la shared memory. No destruye
 *              la shared memory. Debe destruirse llamando a "free()"
 * 
 *  @arg:       None.
 * 
 *  @return:    None.
 * ***************************************************************************/
template <class arg_t>
SharedMemory<arg_t>::~SharedMemory()
{
    shmdt( (void *) this->shmaddr);
}


/******************************************************************************
 *  @brief:     OPERATOR=. Escribe en la primera posición.
 * 
 *  @arg:       element: elemento a escribir.
 * 
 *  @return:    Void.
 * ***************************************************************************/
template <class arg_t>
void SharedMemory<arg_t>::operator= (arg_t element)
{
    this->write(element, 0);
}

/******************************************************************************
 *  @brief:     OPERATOR<<. Escribe en la primera posición.
 * 
 *  @arg:       element: elemento a escribir.
 * 
 *  @return:    Void.
 * ***************************************************************************/
template <class arg_t>
SharedMemory<arg_t>& SharedMemory<arg_t>::operator<< (arg_t element)
{
    this->write(element, 0);
    return *this;
}

/******************************************************************************
 *  @brief:     OPERATOR[]. Devuelve UNA REFERENCIA al elemento en "index".
 * 
 *  @arg:       index: posición en la shared memory.
 * 
 *  @return:    UNA REFERENCIA al valor.
 * ***************************************************************************/
template <class arg_t>
arg_t& SharedMemory<arg_t>::operator[] (int index)
{
    return this->shmaddr[index];
} 

#endif //SHARED_MEM_H