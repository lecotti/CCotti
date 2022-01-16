#include "sig.h"
#include "gtest/gtest.h"

#include <iostream>
#include <sys/types.h>
#include <unistd.h>

static int volatile g_sig;

class SignalTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        g_sig = 0;
    }
};

void add_handler (int signal);
void BlockInHandler_handler_first (int signal);
void BlockInHandler_handler_second (int signal);


/******************************************************************************
 * Being tested: Signal::kill(), Signal::set_handler().
 * 
 * Expected result: Signal handlers should be activated when the indicated
 * signal is received.
 *****************************************************************************/
TEST_F(SignalTest, SendSignal)
{
    Signal::set_handler(SIGUSR1, add_handler);
    Signal::set_handler(SIGUSR2, add_handler);

    Signal::kill(getpid(), SIGUSR1);
    EXPECT_EQ(g_sig, SIGUSR1);

    Signal::kill(getpid(), SIGUSR2);
    EXPECT_EQ(g_sig, SIGUSR1 + SIGUSR2);
}

/******************************************************************************
 * Being tested: Signal::block(), Signal::unblock()
 * 
 * Expected result: When a signal is blocked, it's handler should not be
 * called, but after being unblocked, it's handler should be called only once
 * if the signal was received.
 *****************************************************************************/
TEST_F(SignalTest, BlockSignal)
{
    Signal::set_handler(SIGUSR1, add_handler);
    Signal::set_handler(SIGUSR2, add_handler);
    Signal::block(SIGUSR1);
    Signal::block(SIGUSR2);

    Signal::kill(getpid(), SIGUSR1);
    Signal::kill(getpid(), SIGUSR1);
    Signal::kill(getpid(), SIGUSR1);

    EXPECT_EQ(g_sig, 0);

    Signal::unblock(SIGUSR1);
    EXPECT_EQ(g_sig, SIGUSR1);

    Signal::kill(getpid(), SIGUSR2);
    EXPECT_EQ(g_sig, SIGUSR1);

    Signal::unblock(SIGUSR2);
    EXPECT_EQ(g_sig, SIGUSR1 + SIGUSR2);
}


/******************************************************************************
 * Being tested: unblock_all()
 * 
 * Expected result: When a signal is blocked, it's handler should not be
 * called, but after being unblocked, it's handler should be called only once
 * if the signal was received.
 *****************************************************************************/
TEST_F(SignalTest, UnblockAll)
{
    Signal::block(SIGUSR1);
    Signal::block(SIGINT);
    Signal::block(SIGUSR2);

    Signal::set_handler(SIGUSR1, add_handler);
    Signal::set_handler(SIGUSR2, add_handler);
    Signal::set_handler(SIGINT, add_handler);

    Signal::kill(getpid(), SIGUSR1);
    Signal::kill(getpid(), SIGUSR2);
    Signal::kill(getpid(), SIGINT);

    Signal::unblock_all();

    EXPECT_EQ(g_sig, SIGUSR1 + SIGUSR2 + SIGINT);
}

/******************************************************************************
 * Being tested: Signal::ignore(), Signal::set_default_handler()
 * 
 * Expected result: When ignored, a signal should not be catched, even after
 * being allowed again.
 *****************************************************************************/
TEST_F(SignalTest, IgnoreSignal)
{
    Signal::ignore(SIGINT);
    Signal::kill(getpid(), SIGINT);

    Signal::set_default_handler(SIGINT);
    EXPECT_EXIT(Signal::kill(getpid(), SIGINT), ::testing::KilledBySignal(SIGINT), "");
}


/******************************************************************************
 * Being tested: Signal::set_handler(to_block)
 * 
 * Expected result: A signal handler should be able to block certain signals,
 * and their handles be called only after the first signal handler terminates.
 *****************************************************************************/
TEST_F(SignalTest, BlockInHandler)
{
    int to_block[] = {SIGUSR2, SIGINT};
    Signal::set_handler(SIGUSR1, BlockInHandler_handler_first, 0, to_block, 2); // Signals blocked in handler.
    Signal::set_handler(SIGUSR2, add_handler);
    Signal::set_handler(SIGINT, add_handler);

    Signal::kill(getpid(), SIGUSR1);

    EXPECT_EQ(g_sig, SIGUSR2 + SIGINT);

    g_sig = 0;

    Signal::set_handler(SIGUSR1, BlockInHandler_handler_second); // Now both handlers should be called inside the other handler.

    Signal::kill(getpid(), SIGUSR1);

    EXPECT_EQ(g_sig, SIGUSR2 + SIGINT);
}

/******************************************************************************
 * Being tested: Signal::wait_and_handle(), Signal::set_alarm()
 * 
 * Expected result: The handler should be called after the wait.
 *****************************************************************************/
TEST_F(SignalTest, WaitAndHandle)
{
    Signal::set_handler(SIGALRM, add_handler);
    Signal::set_alarm(1);
    Signal::wait_and_handle(SIGALRM);
    EXPECT_EQ(g_sig, SIGALRM);
}

/******************************************************************************
 * Being tested: Signal::wait_and_continue(), Signal::set_alarm()
 * 
 * Expected result: The handler should NOT be called after the wait.
 *****************************************************************************/
TEST_F(SignalTest, WainAndContinue)
{
    Signal::set_handler(SIGALRM, add_handler);
    Signal::set_alarm(1);
    Signal::wait_and_continue(SIGALRM);
    EXPECT_EQ(g_sig, 0);
}


/******************************************************************************
 * AUXILIARY FUNCTIONS
 *****************************************************************************/
void add_handler (int signal)
{
    g_sig += signal;
}

void BlockInHandler_handler_first (int signal)
{
    Signal::kill(getpid(), SIGUSR2);

    EXPECT_EQ(g_sig, 0);

    Signal::kill(getpid(), SIGINT);

    EXPECT_EQ(g_sig, 0);
}

void BlockInHandler_handler_second (int signal)
{
    Signal::kill(getpid(), SIGUSR2);

    EXPECT_EQ(g_sig, SIGUSR2);

    Signal::kill(getpid(), SIGINT);

    EXPECT_EQ(g_sig, SIGUSR2 + SIGINT);
}