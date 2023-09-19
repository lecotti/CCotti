#include "sig.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

/******************************************************************************
 * Test auxiliary definitions
******************************************************************************/

static int volatile g_signal_value;

class SignalTest: public ::testing::Test {
protected:
    void SetUp() override {
        g_signal_value = 0;
    }
};

void addition_handler (int signal);
void BlockInHandler1 (int signal);
void BlockInHandler2 (int signal);

void addition_handler (int signal) {
    g_signal_value += signal;
}

void BlockInHandler1(int signal) {
    Signal::kill(getpid(), SIGUSR2);
    EXPECT_EQ(g_signal_value, 0);
    Signal::kill(getpid(), SIGINT);
    EXPECT_EQ(g_signal_value, 0);
}

void BlockInHandler2 (int signal) {
    Signal::kill(getpid(), SIGUSR2);
    EXPECT_EQ(g_signal_value, SIGUSR2);
    Signal::kill(getpid(), SIGINT);
    EXPECT_EQ(g_signal_value, SIGUSR2 + SIGINT);
}

/******************************************************************************
 * Tests
******************************************************************************/

/// @brief Tested: Signal::kill(), Signal::set_handler().
TEST_F(SignalTest, SendSignal) {
    Signal::set_handler(SIGUSR1, addition_handler);
    Signal::set_handler(SIGUSR2, addition_handler);
    Signal::kill(getpid(), SIGUSR1);
    EXPECT_EQ(g_signal_value, SIGUSR1);
    Signal::kill(getpid(), SIGUSR2);
    EXPECT_EQ(g_signal_value, SIGUSR1 + SIGUSR2);
}

/// @brief Tested: Signal::block(), Signal::unblock()
TEST_F(SignalTest, BlockSignal) {
    Signal::set_handler(SIGUSR1, addition_handler);
    Signal::set_handler(SIGUSR2, addition_handler);
    Signal::block(SIGUSR1);
    Signal::block(SIGUSR2);
    Signal::kill(getpid(), SIGUSR1);
    Signal::kill(getpid(), SIGUSR1);
    Signal::kill(getpid(), SIGUSR1);
    EXPECT_EQ(g_signal_value, 0);
    Signal::unblock(SIGUSR1);
    EXPECT_EQ(g_signal_value, SIGUSR1);
    Signal::kill(getpid(), SIGUSR2);
    EXPECT_EQ(g_signal_value, SIGUSR1);
    Signal::unblock(SIGUSR2);
    EXPECT_EQ(g_signal_value, SIGUSR1 + SIGUSR2);
}

/// @brief Tested: unblock_all()
TEST_F(SignalTest, UnblockAll) {
    Signal::block(SIGUSR1);
    Signal::block(SIGINT);
    Signal::block(SIGUSR2);
    Signal::set_handler(SIGUSR1, addition_handler);
    Signal::set_handler(SIGUSR2, addition_handler);
    Signal::set_handler(SIGINT, addition_handler);
    Signal::kill(getpid(), SIGUSR1);
    Signal::kill(getpid(), SIGUSR2);
    Signal::kill(getpid(), SIGINT);
    Signal::unblock_all();
    EXPECT_EQ(g_signal_value, SIGUSR1 + SIGUSR2 + SIGINT);
}

/// @brief Tested: Signal::ignore(), Signal::set_default_handler()
TEST_F(SignalTest, IgnoreSignal) {
    Signal::ignore(SIGINT);
    Signal::kill(getpid(), SIGINT); // This signal should be ignored
    Signal::set_default_handler(SIGINT);
    EXPECT_EXIT(Signal::kill(getpid(), SIGINT), ::testing::KilledBySignal(SIGINT), "");
}

/// @brief Tested: Signal::set_handler(), with "signals_blocked_in_handler" argument.
TEST_F(SignalTest, BlockInHandler) {
    int signals_blocked_in_handler[] = {SIGUSR2, SIGINT};
    // Handlers are not called until exiting the handler.
    Signal::set_handler(SIGUSR1, BlockInHandler1, 0, signals_blocked_in_handler, 2);
    Signal::set_handler(SIGUSR2, addition_handler);
    Signal::set_handler(SIGINT, addition_handler);
    Signal::kill(getpid(), SIGUSR1);
    EXPECT_EQ(g_signal_value, SIGUSR2 + SIGINT);
    g_signal_value = 0;
    // Now both handlers should be called inside this handler.
    Signal::set_handler(SIGUSR1, BlockInHandler2);
    Signal::kill(getpid(), SIGUSR1);
    EXPECT_EQ(g_signal_value, SIGUSR2 + SIGINT);
}

/// @brief Tested: Signal::wait(), Signal::set_timer_one_shot()
TEST_F(SignalTest, Wait) {
    Signal::set_handler(SIGALRM, addition_handler);
    Signal::set_timer_single_shot(5);
    EXPECT_LT(Signal::get_timer_time(), 5);
    Signal::wait(SIGALRM);
    EXPECT_EQ(Signal::get_timer_time(), 0);
    EXPECT_EQ(g_signal_value, SIGALRM);
}

/// @brief Tested: Signal::wait_and_ignore(), Signal::set_timer_one_shot()
TEST_F(SignalTest, WainAndIgnore) {
    Signal::set_handler(SIGALRM, addition_handler);
    Signal::set_timer_single_shot(5);
    Signal::wait_and_ignore(SIGALRM);
    EXPECT_EQ(g_signal_value, 0);
}

/// @brief Tested: Signal::set_timer_periodic(), and also correct timing.
TEST_F(SignalTest, TimerPeriodic) {
    Signal::set_handler(SIGALRM, addition_handler);
    Signal::set_timer_periodic(30);
    bool three, two, one;
    for (uint8_t i = 0; i < 3; i++) {
        three=false; two=false; one = false;
        while(!(three && two && one && g_signal_value != SIGALRM*i) ) {
            if(Signal::get_timer_time() > 20 && Signal::get_timer_time() < 30) {
                three = true;
            } else if(Signal::get_timer_time() > 10 && Signal::get_timer_time() < 20) {
                two = true;
            } else if(Signal::get_timer_time() > 0 && Signal::get_timer_time() < 10) {
                one = true;
            }
        }
        printf("\nSignal: %d\n", g_signal_value);
        EXPECT_EQ(g_signal_value, SIGALRM*(i+1));
    }
    Signal::unset_timer();
}
