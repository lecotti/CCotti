#ifndef TOOLS_H
#define TOOLS_H

#define RESET               "\x1b[0m"
#define RED(str)            "\x1b[31m" str RESET
#define GREEN(str)          "\x1b[32m" str RESET
#define YELLOW(str)         "\x1b[33m" str RESET
#define BLUE(str)           "\x1b[34m" str RESET
#define MAGENTA(str)        "\x1b[35m" str RESET
#define CYAN(str)           "\x1b[36m" str RESET

#define OK(str)         GREEN("[ OK ] ") str
#define DEBUG(str)      MAGENTA("[ DEBUG ] ") str
#define ERROR(str)      RED("[ ERROR ] ") str
#define WARNING(str)    YELLOW("[ WARNING ] ") str
#define INFO(str)       CYAN("[ INFO ] ") str

#endif //TOOLS_H
