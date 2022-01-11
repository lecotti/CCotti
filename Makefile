###############################################################################
#	USER DEFINED VARIABLES
###############################################################################

#Este es el nombre del archivo .cpp en la carpeta src.
#El archivo de testeo debe llamarse "test_$(SRC).cpp"
SRC = msg_queue

###############################################################################
#	DEVELOPER VARIABLES
###############################################################################

CC = g++

BUILD_PATH = ./build/

SRC_PATH = ./src/

TEST_SRC_PATH = ./tests/

OBJ_FILES = $(BUILD_PATH)$(SRC).o $(BUILD_PATH)test_$(SRC).o $(BUILD_PATH)sem.o


exe.out: $(OBJ_FILES)

	$(CC) $^ -o $@ -Wall -pthread

${BUILD_PATH}%.o: $(SRC_PATH)%.cpp

	$(CC) $< -c -Wall -pthread -o $@

${BUILD_PATH}%.o: $(TEST_SRC_PATH)%.cpp

	$(CC) $< -c -Wall -pthread -o $@



clear:
	rm $(wildcard $(BUILD_PATH)*.o)
	rm $(wildcard *.out)
	rm $(wildcard *.txt)