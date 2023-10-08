#ifndef H_HTTP_SERVER
#define H_HTTP_SERVER

#include "server.h"
#include "tools.h"
#include "sig.h"
#include "sem.h"
#include "shared_memory.h"
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "http_types.h"
#include "temp_sensor.h"

#define PID_FILE    "/tmp/cotti_server_id"

#define DEFAULT_BACKLOG 2
#define DEFAULT_MAX_CLIENTS 1000
#define DEFAULT_SENSOR_PERIOD 1000
#define DEFAULT_SAMPLES_MOVING_AVERAGE_FILTER 5
#define SERVER_ROOT "web"
#define REQUEST_SIZE 2048
#define RESPONSE_SIZE 2048
#define CONFIG_FILE_PATH_SIZE 256
#define SERVER_NAME "Cotti_server"

class HttpServer: public Server {
protected:
    static bool flag_update_conf;
    SharedMemory<serverData> shm;
    Sem sem;
    TempSensor temp_sensor;
    char config_file[CONFIG_FILE_PATH_SIZE];

    void on_start(void) override;
    void on_accept(Socket& socket) override;
    int request(Socket& socket, HttpRequest* req);
    int response(Socket& socket, HttpResponse res);
    static void sigusr1_handler(int signal);
    void update_configuration(void);

    // Responses
    HttpResponse response_bad_request(void);
    HttpResponse response_update(void);
    HttpResponse response_404_image(void);
    HttpResponse response_max_clients_image(void);
    HttpResponse response_root(void);
    HttpResponse response_favicon(void);
    HttpResponse response_max_clients(void);
    HttpResponse response_not_found(void);

public:
    HttpServer(const char* ip=NULL, const char* port="http", const char* config_file="config.cfg");
};

#endif // H_HTTP_SERVER