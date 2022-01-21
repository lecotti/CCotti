#include "msg_queue.h"
#include "gtest/gtest.h"
#include <sys/wait.h>
#include <string.h>

/******************************************************************************
 * Being tested: All IO operations with int type.
 * 
 * Expected result: Correct functionality.
 *****************************************************************************/
TEST (MsgQueueTest, IntType)
{
    MsgQueue<int> queue (".", 2, true);

    for (int i = 0; i < 10; i++)
    {
        queue << i; //Writing with operator.
    }

    if (!fork())    // child
    {
        int child_value = 0;
        int read;
        MsgQueue<int> child_queue (".", 2);

        for (int i = 0; i < 10; i++)
        {
            child_queue >> read;    //Reading with operator.
            EXPECT_EQ(i, read);
            child_value += read;
        }
        child_queue << child_value;
    }

    else    // father
    {
        wait(NULL);
    
        EXPECT_EQ(queue.read(), 45);    //Reading with function

        EXPECT_TRUE(MsgQueue<int>::exists(".", 2));
        queue.free();
        EXPECT_FALSE(MsgQueue<int>::exists(".", 2));
    }
}


/******************************************************************************
 * Being tested: All IO operations with a struct type.
 * 
 * Expected result: Correct functionality.
 *****************************************************************************/
TEST(MsgQueueTest, StructType)
{
    struct person
    {
        char name[20];
        int id;
    };

    struct person data;
    data.id = 10;
    strcpy(data.name, "nico");

    MsgQueue<struct person> queue (".", 2, true);

    queue.write(data);

    if (!fork())    // child
    {
        struct person child_data;
        MsgQueue<struct person> child_queue (".", 2);

        child_queue >> child_data;
        strcat (child_data.name, "las");
        child_data.id += 10;
        child_queue << child_data;
    }

    else    // father
    {
        wait(NULL);
        data = queue.read();

        EXPECT_EQ(data.id, 20);
        EXPECT_STREQ(data.name, "nicolas");

        EXPECT_TRUE(MsgQueue<int>::exists(".", 2));
        queue.free();
        EXPECT_FALSE(MsgQueue<int>::exists(".", 2));
    }
}

/******************************************************************************
 * Being tested: MsgQueue::MsgQueue()
 * 
 * Expected result: I shouldn't be able to create two equal shared memories.
 *  If none exist, i should be able to connect.
 *****************************************************************************/
TEST(MsgQueueTest, Creation)
{
    EXPECT_FALSE(MsgQueue<int>::exists(".", 2));

    MsgQueue<int> queue(".", 2, true);

    EXPECT_TRUE(MsgQueue<int>::exists(".", 2));
    
    EXPECT_THROW(MsgQueue<int>(".", 2, true), std::runtime_error);

    queue.free();

    EXPECT_THROW(MsgQueue<int>(".", 2), std::runtime_error);
}

/******************************************************************************
 * Being tested: MsgQueue::peek()
 * 
 * Expected result: Non destructive read from the queue.
 *****************************************************************************/
TEST(MsgQueueTest, PeekTest)
{
    MsgQueue<int> queue(".", 2, true);
    int status = 0;
    queue << 10;
    queue << 11;
    queue << 12;

    EXPECT_EQ(queue.peek(0), 10);
    EXPECT_EQ(queue.peek(1), 11);
    EXPECT_EQ(queue.peek(2), 12);

    queue.peek(3, &status); // Expected error, no fourth message, status = -1.

    EXPECT_EQ(status, -1);

    EXPECT_EQ(queue.read(), 10);
    EXPECT_EQ(queue.read(), 11);
    EXPECT_EQ(queue.read(), 12);

    queue.free();
}

/******************************************************************************
 * Being tested: get_msg_qtty(), is_empty(), has_msg(). 
 * 
 * Expected result: To know the exact amount of msg in a queue.
 *****************************************************************************/
TEST(MsgQueueTest, QttyTest)
{
    MsgQueue<int> queue(".", 2, true);

    EXPECT_TRUE(queue.is_empty());
    EXPECT_FALSE(queue.has_msg());

    queue << 0;
    queue << 1;
    queue << 2;

    EXPECT_EQ(queue.get_msg_qtty(), 3);

    queue.read();
    queue.read();

    EXPECT_FALSE(queue.is_empty());
    EXPECT_TRUE(queue.has_msg());

    queue.read();

    EXPECT_TRUE(queue.is_empty());
    EXPECT_FALSE(queue.has_msg());

    queue.free();

    EXPECT_EQ(queue.get_msg_qtty(), -1);
}

/******************************************************************************
 * Being tested: read() 
 * 
 * Expected result: reading depends on mtype value.
 *****************************************************************************/
TEST(MsgQueueTest, Priority)
{
    MsgQueue<int> queue(".", 2, true);

    queue.write(5, 5);
    queue.write(4, 4);
    queue.write(3, 3);
    queue.write(2, 2);
    queue.write(1, 1);

    EXPECT_EQ(queue.read(), 5);     //El primero de la cola
    EXPECT_EQ(queue.read(3), 3);    //Tengo que sacar el 3
    EXPECT_EQ(queue.read(1), 1);   
    EXPECT_EQ(queue.read(-4), 2);   //Tiene que leer el que tenga mtype lo más chico posible, osea 2
    EXPECT_EQ(queue.read(), 4);     //El primero

    queue.free();
}