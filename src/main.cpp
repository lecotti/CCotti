#include "main.h"

int main(void) {
    HttpServer server("localhost", "3000", "config.cfg");
    server.start();
    return 0;
}
