#include "server.h"
#include "tools.h"
#include "sig.h"
#include "sem.h"
#include <string.h>

typedef struct HttpRequest {
    char route[100];
} HttpRequest;

typedef struct HttpResponse {
    char route[100];
    char mime_type[100];
    int http_code;
    char http_status[100];
} HttpResponse;

class HttpServer: public Server {
protected:
    static bool flag_update_conf;
    static Sem sem;

    // Parameters from the configuration archive
    int backlog;
    int max_clients;
    int sensor_period;
    int samples_moving_average_filter;

    int client_count;
    char config_file[256];

    void on_start(void) override;
    void on_accept(Socket& socket) override;
    void on_new_client(void) override;
    static void sigusr1_handler(int signal);    // Updated text file
    static void sigusr2_handler(int signal);    // Connection closed
    void response(Socket& socket, HttpResponse res);
    int request(Socket& socket, HttpRequest* req);
    HttpResponse not_found(void);

public:
    HttpServer(const char* ip, const char* port, const char* config_file="config.cfg");
    void update_configuration(void);
};

#define DEFAULT_BACKLOG 2
#define DEFAULT_MAX_CLIENTS 1000
#define DEFAULT_SENSOR_PERIOD 1000
#define DEFAULT_SAMPLES_MOVING_AVERAGE_FILTER 5
#define SERVER_ROOT "web"