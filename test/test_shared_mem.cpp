#include "shared_memory.h"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

/// @brief Tested: SharedMemory::ShareMemory(), SharedMemory::exists()
TEST(SharedMemTest, Creation) {
    EXPECT_THROW(SharedMemory<int>(".", 2), std::runtime_error);
    EXPECT_FALSE(SharedMemory<int>::exists(".", 2));
    SharedMemory<int>shm(".", 2, 5);
    EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
    EXPECT_THROW(SharedMemory<int>(".", 2, 10), std::runtime_error);
}

/// @brief Tested: all writing and reading methods. with int type.
TEST(SharedMemoryTest, IntegerType) {
    SharedMemory<int> shm(".", 2, 4);
    int array[] = {11, 12};
    shm << 10;                  // First element write.
    shm.write(array, 2, 1);     // Array write
    shm.write(13, 3);           // One element write.
    if (!fork()) {
        // Child
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
        SharedMemory<int>child_shm(".", 2);
        int child_array[2] = {0, 0};
        child_shm[0] += 10;                 // Reference writing
        EXPECT_EQ(child_shm[0], 20);
        child_shm.read(child_array, 2, 1);  // Array reading
        EXPECT_EQ(child_array[0], 11);
        EXPECT_EQ(child_array[1], 12);
        child_array[0] += 10;
        child_array[1] += 10;
        child_shm.write(child_array, 2, 1); // Array writing
        EXPECT_EQ(child_shm.read(3), 13);
        child_shm.write(child_shm.read(3) + 10, 3);
        EXPECT_EQ(child_shm.read(3), 23);
    } else {
        // Father
        wait(NULL);
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
        shm.read(array, 2, 1);
        EXPECT_EQ(shm.read(0), 20);
        EXPECT_EQ(array[0], 21);
        EXPECT_EQ(array[1], 22);
        EXPECT_EQ(shm[3], 23);
    }
}

/// @brief Tested: All writing and reading methods with char type.
TEST(SharedMemoryTest, CharType) {
    SharedMemory<char> shm(".", 2, 40);
    char value1[20] = "aa";
    char value2[20] = "bb";
    shm.write(value1, 20, 0);
    shm.write(value2, 20, 20);
    if (!fork()) {
        // Child
        SharedMemory<char> child_shm(".", 2);
        strcat(&(child_shm[0]), "0");
        strcat(&(child_shm[20]), "1");
    } else {
        // Father
        wait(NULL);
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
        shm.read(value1, 20, 0);
        shm.read(value2, 20, 20);
        EXPECT_STREQ(value1, "aa0" );
        EXPECT_STREQ(value2, "bb1");
    }
}

/// @brief Tested: All writing and reading methods with custom struct type.
TEST(SharedMemoryTest, StructType) {
    struct person {
        int id;
        char name[20];
    };
    SharedMemory<struct person>shm(".", 2, 2);
    char value1[20] = "xxx";
    char value2[20] = "zzz";
    struct person data[2];
    data[0].id = 10;
    data[1].id = 11;
    strcpy(data[0].name, value1);
    strcpy(data[1].name, value2);
    shm << data[0];
    shm.write(data[1], 1);
    if (!fork()) {
        SharedMemory<struct person> child_shm(".", 2);
        shm[0].id += 10;
        shm[1].id += 10;
        strcat(child_shm[0].name, "0");     // Reference writing
        strcat(child_shm[1].name, "1");
    } else {
        // Father
        wait(NULL);
        EXPECT_TRUE(SharedMemory<int>::exists(".", 2));
        EXPECT_EQ(shm.read(0).id, 20);
        EXPECT_EQ(shm[1].id, 21);
        EXPECT_STREQ(shm.read(0).name, "xxx0" );
        EXPECT_STREQ(shm[1].name, "zzz1");
    }
}
