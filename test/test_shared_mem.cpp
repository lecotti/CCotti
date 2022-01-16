#include "shared_memory.h"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


/******************************************************************************
 * Being tested: SharedMemory::ShareMemory(), SharedMemory::exists()
 * 
 * Expected result: shm should be created and destroyed correctly. Can't be
 * created twice, and if it doesn't exists you shouldn't be able to connect to.
 *****************************************************************************/
TEST(SharedMemTest, Creation)
{
    SharedMemory<int> shm(".", 2, 5);

    EXPECT_TRUE(SharedMemory<int>::exists(".", 2));

    EXPECT_THROW(SharedMemory<int>(".", 2, 10), std::runtime_error);

    shm.free();

    EXPECT_THROW(SharedMemory<int>(".", 2), std::runtime_error);

    EXPECT_FALSE(SharedMemory<int>::exists(".", 2));
}

/******************************************************************************
 * Being tested: All writing and reading methods. with int type.
 * 
 * Expected result: Correct IO operations.
 *****************************************************************************/
TEST(SharedMemoryTest, IntegerType)
{
    SharedMemory<int> shm(".", 2, 4);
    int array[] = {11, 12};

    shm << 10;                  // First element write.
    shm.write(array, 2, 1);     // Array write
    shm.write(13, 3);           // One element write.
    
    
    if (!fork())
    {
        SharedMemory<int> child_shm(".", 2);
        int child_array[2] = {0, 0};
        int value;

        child_shm[0] += 10;     // Reference writing
    
        child_shm.read(child_array, 2, 1);  //Array read

        child_array[0] += 10;
        child_array[1] += 10;
        
        child_shm.write(child_array, 2, 1); // Array writing

        child_shm.write(child_shm.read(3) + 10, 3);
        exit(1);
    }

    else
    {
        //Father
        wait(NULL);
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));

        shm.read(array, 2, 1);
        EXPECT_EQ(shm.read(0), 20 );
        EXPECT_EQ(array[0], 21);
        EXPECT_EQ(array[1], 22);
        EXPECT_EQ(shm[3], 23);

        shm.free();
        EXPECT_FALSE(SharedMemory<int>::exists(".", 2));
    }
}


/******************************************************************************
 * Being tested: All writing and reading methods with char type.
 * 
 * Expected result: Correct IO operations.
 *****************************************************************************/
TEST(SharedMemoryTest, CharType)
{
    //The unspoken rule is that every string will be at most 20 chars long, including nul.
    //So we have "N" strings of 20 chars long.
    SharedMemory<char> shm(".", 2, 40);
    char value1[20] = "aa";
    char value2[20] = "bb";

    shm.write(value1, 20, 0);
    shm.write(value2, 20, 20);    

    if (!fork())
    {
        SharedMemory<char> child_shm(".", 2);

        strcat(&(child_shm[0]), "0");
        strcat(&(child_shm[20]), "1");
        exit(1);
    }

    else
    {
        //Father
        wait(NULL);
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
        shm.read(value1, 20, 0);
        shm.read(value2, 20, 20);
        EXPECT_STREQ(value1, "aa0" );
        EXPECT_STREQ(value2, "bb1");


        shm.free();
        EXPECT_FALSE(SharedMemory<int>::exists(".", 2));
    }
}

/******************************************************************************
 * Being tested: All writing and reading methods with custom struct type.
 * 
 * Expected result: Correct IO operations.
 *****************************************************************************/
TEST(SharedMemoryTest, StructType)
{
    struct person
    {
        int id;
        char name[20];
    };

    SharedMemory<struct person> shm(".", 2, 2);
    char value1[20] = "xxx";
    char value2[20] = "zzz";

    struct person data[2];
    data[0].id = 10;
    data[1].id = 11;
    strcpy(data[0].name, value1);
    strcpy(data[1].name, value2);

    shm << data[0];
    shm.write(data[1], 1);


    if (!fork())
    {
        SharedMemory<struct person> child_shm(".", 2);

        shm[0].id += 10;
        shm[1].id += 10;
        strcat(child_shm[0].name, "0");     // Reference writing
        strcat(child_shm[1].name, "1");
        exit(1);
    }

    else
    {
        //Father
        wait(NULL);
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
        EXPECT_EQ(shm.read(0).id, 20);
        EXPECT_EQ(shm[1].id, 21);
        EXPECT_STREQ(shm.read(0).name, "xxx0" );
        EXPECT_STREQ(shm[1].name, "zzz1");

        shm.free();
        EXPECT_FALSE(SharedMemory<int>::exists(".", 2));
    }

}