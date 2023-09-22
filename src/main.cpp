#include "main.h"

int main(void) {
    HttpServer server(NULL, "3000", "config.cfg");
    server.start();
    return 0;
}
