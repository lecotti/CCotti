#ifndef SEM_H
#define SEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "tools.h"
#include <stdexcept>

class Sem {
private:
    int semid;
    bool creator;

public:
    Sem(const char* path, int id, bool create=false);
    ~Sem();
    static bool exists(const char* path, int id);

    int set(int value);
    int get() const;
    int op (int op);

    int operator++ (int);
    int operator++ ();
    int operator-- (int);
    int operator-- ();
    int operator= (int a);
    int operator+ (int a);
    int operator- (int a);
};

#endif //SEM_H