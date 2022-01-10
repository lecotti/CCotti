#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

template <class arg_t>
class SharedMemory
{
private:
    int shmid;
    arg_t* shmaddr;

public:

    SharedMemory(int size, const char* path, int id);

    void write( arg_t* elements, int size, int index=0);
    void write(arg_t element, int index);
    
    arg_t* read(arg_t* array, int size, int index=0);
    arg_t& read(int index);

    int free(void);

    ~SharedMemory();

    void operator= (arg_t element);
    arg_t operator<< (arg_t element);
    arg_t& operator[] (int index);
};

#endif //SHARED_MEMORY_H


/******************************************************************************
 *  @brief:     Crea o instancia una shared memory: un espacio de memoria compartible
 *              entre procesos. Se representa como un puntero del tipo <arg_t>, de
 *              tamaño size.
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
 *  @return:    None.
 * ***************************************************************************/
template <class arg_t>
SharedMemory<arg_t>::SharedMemory(int size, const char* path, int id)
{
    key_t key;
    if ( (key = ftok(path, id) ) != -1)
    {
        if (size)
        {
            if( (this->shmid = shmget(key, (size_t)size*sizeof(arg_t), IPC_CREAT | 0666) ) == -1)
            {
                perror(RED("shmget.\n"));
                return;
            }
        }

        else
        {
            if( (this->shmid = shmget(key, 0, 0) ) == -1)
            {
                perror(RED("shmget: shared_memory doesn't exist.\n"));
                return;
            }
        }
        

        if ( (this->shmaddr = (arg_t*) shmat(this->shmid, NULL, 0)) == (arg_t*) -1)
        {
            perror(RED("shmat.\n"));
            return;
        }
    }

    else
    {
        perror(RED("ftok.\n"));
        return;
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
arg_t& SharedMemory<arg_t>::read(int index)
{
    return this->shmaddr[index];
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
    shmdt((void *) this->shmaddr);
    return shmctl(this->shmid, IPC_RMID, NULL);
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
arg_t SharedMemory<arg_t>::operator<< (arg_t element)
{
    this->write(element, 0);
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