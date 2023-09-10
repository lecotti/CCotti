#include "sem.h"
#include "gtest/gtest.h"
#include <sys/wait.h>

/// @brief Tested: exists(), Sem:Sem()
TEST(SemTest, Creation) {
    EXPECT_FALSE(Sem::exists(".", 2));
    Sem sem(".", 2, true);
    EXPECT_TRUE(Sem::exists(".", 2));
    EXPECT_THROW(Sem(".", 2, true), std::runtime_error);
    Sem sem_child(".", 2);
}

/// @brief Tested: operators ++, --, +, -, =.
TEST(SemTest, ValueAssignment) {
    Sem sem(".", 2, true);
    EXPECT_EQ(sem.get(), 1);
    EXPECT_EQ(++sem, 2);
    EXPECT_EQ(sem = sem + 5, 7);
    sem = sem - 4;
    EXPECT_EQ(sem.get(), 3);
    EXPECT_EQ(sem.get(), 3);
    sem = 6;
    EXPECT_EQ(sem.get(), 6);
    sem--;
    EXPECT_EQ(sem.get(), 5);
}

/// @brief Tested: op(0)
TEST(SemTest, WaitForZero) {
    Sem sem (".", 2, true);
    if (!fork()) {
        //Child
        EXPECT_EQ(Sem::exists(".", 2), true);
        Sem child_sem(".", 2);
        child_sem--;
    } else {
        // Parent should be able to exit.
        wait(NULL);
        EXPECT_TRUE(Sem::exists(".", 2));
        sem.op(0);
    }
}

/// @brief Tested: Proccess sync.
TEST(SemTest, Sync) {
    Sem sem(".", 2, true);
    if (!fork()) {
        //Child
        Sem child_sem(".", 2);
        child_sem--;
        sem = sem + 2;
    } else {
        sem = sem - 2;
        wait(NULL);
        EXPECT_TRUE(Sem::exists(".", 2));
    }
}