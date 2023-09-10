#include "msg_queue.h"
#include "gtest/gtest.h"
#include <sys/wait.h>
#include <string.h>

/// @brief Tested: IO operations with int type.
TEST (MsgQueueTest, IntType) {
    MsgQueue<int>queue(".", 2, true);
    for (uint8_t i = 0; i < 10; i++) {
        queue << i; // Writing with operator.
    }
    if (!fork()) {
        // Child
        int sum_of_values = 0;
        int value_read;
        MsgQueue<int>child_queue(".", 2);
        for (int i = 0; i < 10; i++) {
            child_queue >> value_read;    //Reading with operator.
            EXPECT_EQ(i, value_read);
            sum_of_values += value_read;
        }
        child_queue << sum_of_values;
    } else {
        wait(NULL);
        EXPECT_EQ(queue.read(), 45);    //Reading with function
        EXPECT_TRUE(MsgQueue<int>::exists(".", 2));
    }
}

/// @brief Tested: All IO operations with a struct type.
TEST(MsgQueueTest, StructType) {
    struct person {
        char name[20];
        int id;
    };
    struct person data;
    data.id = 10;
    strcpy(data.name, "co");
    MsgQueue<struct person>queue(".", 2, true);
    queue.write(data);
    if (!fork()) {
        // child
        struct person child_data;
        MsgQueue<struct person>child_queue(".", 2);
        child_queue >> child_data;
        strcat(child_data.name, "tti");
        child_data.id += 10;
        child_queue << child_data;
    } else {
        wait(NULL);
        data = queue.read();
        EXPECT_EQ(data.id, 20);
        EXPECT_STREQ(data.name, "cotti");
        EXPECT_TRUE(MsgQueue<int>::exists(".", 2));
    }
}

/// @brief Tested: MsgQueue::MsgQueue()
TEST(MsgQueueTest, Creation) {
    EXPECT_FALSE(MsgQueue<int>::exists(".", 2));
    EXPECT_THROW(MsgQueue<int>(".", 2), std::runtime_error);
    MsgQueue<int> queue(".", 2, true);
    EXPECT_TRUE(MsgQueue<int>::exists(".", 2));
    EXPECT_THROW(MsgQueue<int>(".", 2, true), std::runtime_error);
}

/// @brief Tested: MsgQueue::peek()
TEST(MsgQueueTest, PeekTest) {
    MsgQueue<int> queue(".", 2, true);
    int status = 0;
    queue << 10;
    queue << 11;
    queue << 12;
    EXPECT_EQ(queue.peek(0, &status), 10);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(queue.peek(1, &status), 11);
    EXPECT_EQ(queue.peek(2), 12);
    queue.peek(3, &status);
    EXPECT_NE(status, 0);
    EXPECT_EQ(queue.read(), 10);
    EXPECT_EQ(queue.read(), 11);
    EXPECT_EQ(queue.read(), 12);
}

/// @brief Tested: get_msg_qtty(), is_empty(), has_msg().
TEST(MsgQueueTest, QttyTest) {
    MsgQueue<int> queue(".", 2, true);
    EXPECT_TRUE(queue.is_empty());
    EXPECT_FALSE(queue.has_msg());
    queue << 0;
    queue << 1;
    queue << 2;
    EXPECT_EQ(queue.get_msg_qtty(), 3);
    queue.read();
    queue.read();
    EXPECT_EQ(queue.get_msg_qtty(), 1);
    queue.read();
    EXPECT_TRUE(queue.is_empty());
    EXPECT_FALSE(queue.has_msg());
}

/// @brief Tested: read(), with priority
TEST(MsgQueueTest, Priority) {
    MsgQueue<int> queue(".", 2, true);
    queue.write(5, 5);
    queue.write(4, 4);
    queue.write(3, 3);
    queue.write(2, 2);
    queue.write(1, 1);
    EXPECT_EQ(queue.read(), 5);     // Read first.
    EXPECT_EQ(queue.read(3), 3);    // Read third value.
    EXPECT_EQ(queue.read(1), 1);    // Read value with mtype = 1.
    EXPECT_EQ(queue.read(-4), 2);   // Read higher priority available, should read 2.
    EXPECT_EQ(queue.read(), 4);     // First one.
}
