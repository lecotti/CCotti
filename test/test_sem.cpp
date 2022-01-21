#include "sem.h"
#include "gtest/gtest.h"
#include <sys/wait.h>

/******************************************************************************
 * Being tested: exists(), Sem::Sem();
 * 
 * Expected result: Correct creation
 *****************************************************************************/
TEST(SemTest, Creation)
{
    EXPECT_FALSE(Sem::exists(".", 2));
    Sem sem(".", 2, true);

    EXPECT_TRUE(Sem::exists(".", 2));

    EXPECT_THROW(Sem(".", 2, true), std::runtime_error);

    Sem sem_child(".", 2);

    sem.free();

    EXPECT_FALSE(Sem::exists(".", 2));
}


/******************************************************************************
 * Being tested: operators ++, --, +, -, =.
 * 
 * Expected result: Correct addition and substraction from sem value.
 *****************************************************************************/
TEST(SemTest, ValueAssignment)
{
    Sem sem(".", 2, true);

    EXPECT_EQ(sem.get(), 1);

    EXPECT_EQ(++sem, 2);

    EXPECT_EQ(sem + 5, 7);

    EXPECT_EQ(sem - 4, 3);
    EXPECT_EQ(sem.get(), 3);

    sem = 6;

    EXPECT_EQ(sem.get(), 6);

    sem--;

    EXPECT_EQ(sem.get(), 5);

    sem.free();
}

/******************************************************************************
 * Being tested: op(0);
 * 
 * Expected result: The parent should be able to exit.
 *****************************************************************************/
TEST(SemTest, WaitForZero)
{
    Sem sem (".", 2, true);

    if (!fork())
    {
        //Child
        Sem child_sem(".", 2);
        child_sem--;        
    }

    else
    {
        wait(NULL);
        sem.op(0);
        sem.free();
        
    }
}

/******************************************************************************
 * Being tested: syncronizing processes.
 * 
 * Expected result: The parent should be able to exit.
 *****************************************************************************/
TEST(SemTest, Syncro)
{
    Sem sem(".", 2, true);

    if (!fork())
    {
        //Child
        Sem child_sem(".", 2);
        sem--;
        //Doing stuff
        sem + 2;
    }

    else
    {
        sem - 2; //El valor del semáforo es 1, así que se queda bloqueado.
        sem.free();
        wait(NULL);
    }
}