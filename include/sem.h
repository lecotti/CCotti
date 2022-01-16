#ifndef SEM_H
#define SEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "tools.h"

class Sem
{
private:
    int semid;
    int sem_qtty;

public:
    Sem(int sem_qtty, const char* path, int id);

    int set_value (int value);
    int set_value (int value, int semnum);

    int get_value (int semnum=0);

    int op (int op);
    int op (int op, int semnnum);

    int free(void);

    int operator++ (int);
    int operator++ ();
    int operator-- (int);
    int operator-- ();
    void operator= (int a);
    int operator+ (int a);
    int operator- (int a);

};

#endif //SEM_H