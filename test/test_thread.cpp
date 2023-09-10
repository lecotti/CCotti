#include "thread.h"
#include "mutex.h"
#include "gtest/gtest.h"
#include <unistd.h>
#include "tools.h"
#include <stdio.h>
#include "sig.h"

/******************************************************************************
 * Test auxiliary definitions
******************************************************************************/

static int g_value;
Mutex mutex;
Mutex mutex_array[10];

class ThreadTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_value = 0;
    }
};

void* creation_with_mutex_run (void* arg) {
    EXPECT_EQ(mutex.lock(), 0);
    g_value++;
    EXPECT_EQ(mutex.unlock(), 0);
    return NULL;
}

void* args_run (void* arg) {
    mutex.lock();
    g_value += *((int*)arg);
    (*(int*)arg)++;
    mutex.unlock();
    return NULL;
}

void* mutex_run (void* arg) {
    EXPECT_EQ(mutex.trylock(), 0);
    g_value++;
    while (g_value == 1); // Waits until it's modified.
    mutex.unlock();
    g_value = 66;
    return NULL;
}

void* signal_run (void* arg) {
    Signal::wait_and_ignore(SIGUSR1);
    g_value++;
    return NULL;
}

/******************************************************************************
 * Testing functions
******************************************************************************/

/// @brief Tested: Thread::Thread(), Thread::create(), Mutex::lock(),
///  Mutex::unlock() Thread::join()
TEST_F(ThreadTest, Creation) {
    Thread thread[10];
    g_value = 0;
    for (int i = 0; i < 10; i++) {
        thread[i].create(creation_with_mutex_run);
    }
    for (int i = 0; i < 10; i++) {
        thread[i].join();
    }
    EXPECT_EQ(g_value, 10);
}

/// @brief Tested: Thread::create(with args)
TEST_F(ThreadTest, CreationWithArgs) {
    Thread thread[10];
    int arg = 0;
    for (int i = 0; i < 10; i++) {
        thread[i].create(args_run, (void*)&arg);
    }
    for (int i = 0; i < 10; i++){
        thread[i].join();
    }
    EXPECT_EQ(g_value, 45);
    EXPECT_EQ(arg, 10);
}

/// @brief Tested: Mutex::trylock()
TEST_F(ThreadTest, Mutex) {
    Thread thread(mutex_run);
    while (g_value == 0);   // Wait for modification in thread
    EXPECT_EQ(mutex.trylock(), -1);
    g_value++;
    thread.join();
    EXPECT_EQ(g_value, 66);
}

/// @brief Tested: Thread::Thread (detached)
TEST_F(ThreadTest, Detached) {
    Thread thread(creation_with_mutex_run, NULL, true);
    EXPECT_EQ(thread.join(), -1);
    while(g_value == 0);
    EXPECT_EQ(g_value, 1);
}

/// @brief Tested: Thread::send_signal()
TEST_F(ThreadTest, SignalThread) {
    Signal::block(SIGUSR1);         // Signal blocked on main thread
    Thread thread(signal_run);
    thread.send_signal(SIGUSR1);
    thread.join();
    // Signal should not be received from main thread, because it was attended by child thread.
    Signal::unblock(SIGUSR1);
    EXPECT_EQ(g_value, 1);
}
