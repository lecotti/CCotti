#include "main.h"

int main(void) {
    HttpServer server(NULL, SERVER_PORT, CONFIG_FILE);
    server.start();
    return 0;
}
