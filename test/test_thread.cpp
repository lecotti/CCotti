#include "thread.h"
#include "mutex.h"
#include "gtest/gtest.h"
#include <unistd.h>
#include "tools.h"
#include <stdio.h>
#include "sig.h"

static int g_value;
Mutex mutex;
Mutex mutex_array[10];


class ThreadTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        g_value = 0;
    }
};

void* creation_run (void* arg);
void* args_run (void* arg);
void* mutex_run (void* arg);
void* signal_run (void* arg);

/******************************************************************************
 * Being tested: Thread::Thread(), Thread::create(), Mutex::lock(), Mutex::unlock()
 *  Thread::join()
 * 
 * Expected result: Operations made in the thread are seen from the parent.
 *****************************************************************************/
TEST_F(ThreadTest, Creation)
{
    Thread thread[10];
    for (int i = 0; i < 10; i++)
    {
        thread[i].create(creation_run);
    }

    for (int i = 0; i < 10; i++)
    {
        thread[i].join();
    }
    EXPECT_EQ(g_value, 10);
}


/******************************************************************************
 * Being tested: Thread::create(with args)
 * 
 * Expected result: The argument variable, modified by the threads, should change
 *  in the parent.
 *****************************************************************************/
TEST_F(ThreadTest, CreationWithArgs)
{
    Thread thread[10];
    int arg = 0;
    for (int i = 0; i < 10; i++)
    {
        thread[i].create(args_run, (void*)&arg);
    }

    for (int i = 0; i < 10; i++)
    {
        thread[i].join();
    }

    EXPECT_EQ(g_value, 45);
    EXPECT_EQ(arg, 10);
}

/******************************************************************************
 * Being tested: Mutex::trylock()
 * 
 * Expected result: trylock should not block if can't reserve the mutex.
 *****************************************************************************/
TEST_F(ThreadTest, Mutex)
{
    Thread thread(mutex_run);

    while (g_value == 0);

    EXPECT_EQ(mutex.trylock(), -1);
    g_value++;

    thread.join();
    EXPECT_EQ(g_value, 66);
}

/******************************************************************************
 * Being tested: Thread::Thread (detached)
 * 
 * Expected result: Join() should fail, and the thread should keep running.
 *****************************************************************************/
TEST_F(ThreadTest, Detached)
{
    Thread thread(creation_run, NULL, true);
    EXPECT_EQ(thread.join(), -1);
    while(g_value == 0);
    EXPECT_EQ(g_value, 1);
}

/******************************************************************************
 * Being tested: Thread::kill()
 * 
 * Expected result: The signal should be blocked in the main thread, but fetched
 *  by the created thread. Unblocking the signal should NOT terminate the main
 *  thread, as the signal was already fetched.
 *****************************************************************************/
TEST_F(ThreadTest, SignalThread)
{
    Signal::block(SIGUSR1);
    Thread thread(signal_run);
    thread.kill(SIGUSR1);
    thread.join();
    Signal::unblock(SIGUSR1);
    EXPECT_EQ(g_value, 1);
}


/******************************************************************************
 * AUXILIARY FUNCTIONS
 *****************************************************************************/
void* creation_run (void* arg)
{
    mutex.lock();
    g_value++;
    mutex.unlock();
    return NULL;
}

void* args_run (void* arg)
{
    mutex.lock();
    g_value += *((int*)arg);
    (*(int*)arg)++;
    mutex.unlock();
    return NULL;
}

void* mutex_run (void* arg)
{
    EXPECT_EQ(mutex.trylock(), 0);
    g_value++;
    while (g_value == 1); // Waits until it's modified.
    mutex.unlock();
    g_value = 66;
    return NULL;
}

void* signal_run (void* arg)
{
    Signal::wait_and_ignore(SIGUSR1);
    g_value++;
    return NULL;
}