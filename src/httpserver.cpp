#include "httpserver.h"

bool HttpServer::flag_update_conf = true;
Sem HttpServer::sem(".", 2, true);

int counter = 0;

// TODO: los valores de backlog, max_clients, etc
// se clonan para los hijos, y al actualizarlos el servidor no los cambia para los hijos.
// Esos valores deberían leerse desde una shared memory.

HttpServer::HttpServer(const char* ip, const char* port, const char* config_file):
    Server(ip, port) {
    this->backlog = 0;
    this->max_clients = 0;
    this->sensor_period = 0;
    this->samples_moving_average_filter = 0;
    this->client_count = 0;
    strncpy(this->config_file, config_file, sizeof(this->config_file));
    Signal::set_handler(SIGUSR1, HttpServer::sigusr1_handler);
    Signal::set_handler(SIGUSR2, HttpServer::sigusr2_handler);
    printf(INFO("My PID: %d.\n"), getpid());
}

/// @brief When SIGUSR1 is received, the server will update its parameters
///  from the configuration file.
void HttpServer::sigusr1_handler(int signal) {
    HttpServer::flag_update_conf = true;
}

void HttpServer::sigusr2_handler(int signal) {
    //this->flag_client_dc++;
    //sem--;
    //printf(INFO("Client count: %d\n"), --client_count);
    //sem++;
}

/// @brief Before listening or accepting new connections:
///  * Check if the configuration file needs to be updated (SIGUSR1 should )
void HttpServer::on_start(void) {
    if (HttpServer::flag_update_conf) {
        HttpServer::flag_update_conf = false;
        this->update_configuration();
    }
    printf(DEBUG("Hola %d %d\n"), counter++, getpid());
}

void HttpServer::on_new_client(void) {
    //sem--;
    //printf(INFO("Client count: %d\n"), ++client_count);
    //sem++;
}

/// @brief Process a HttpRequest.
/// @param socket The client socket.
/// @param req Where all the request's contents will be stored.
/// @return "0" on success, "-1" on error.
int HttpServer::request(Socket& socket, HttpRequest* req) {
    char client_msg[10000];
    if (socket.read(client_msg, sizeof(client_msg)) > 0) {
        printf("%s\n", client_msg);
        strcpy(req->route, strtok(&(client_msg[strlen("GET ")]), " "));
        return 0;
    }
    return -1;
}

HttpResponse HttpServer::not_found(void) {
    HttpResponse res;
    strcpy(res.route, "/not_found.html");
    strcpy(res.mime_type, "text/html; charset=utf-8");
    res.http_code = 404;
    strcpy(res.http_status, "Not Found");
    return res;
}

void HttpServer::response(Socket& socket, HttpResponse res) {
    char buffer[1000000];
    char rta[1000000];
    long int bytes_read;
    long int header_size;
    if (res.route[0] == '/') {
        FILE* fd;
        strcpy(buffer, SERVER_ROOT);
        strcat(buffer, res.route);
        strcpy(res.route, buffer);

        if ((fd = fopen(res.route, "rb")) == NULL) {
            res = this->not_found();
        } else if ((bytes_read = fread(buffer, 1, sizeof(buffer), fd)) == 0) {
            res = this->not_found();
        }
        fclose(fd);
    } else {
        bytes_read = strlen(res.route);
        strcpy(buffer, res.route);
    }
    sprintf(rta,
        "HTTP/1.1 %d %s\n"
        "Content-Length: %ld\n"
        "Content-Type: %s\n"
        "Connection: Closed\n\n",
        res.http_code, res.http_status, bytes_read, res.mime_type);
    header_size = strlen(rta);
    memcpy(&rta[header_size], buffer, bytes_read);
    socket.write(rta, header_size + bytes_read);
}

void HttpServer::on_accept(Socket& socket) {
    HttpRequest req;
    HttpResponse res;
    while(this->request(socket, &req) == 0) {
        if (strcmp(req.route, "/") == 0) {
            strcpy(res.route, "/index.html");
            strcpy(res.mime_type, "text/html; charset=utf-8");
            res.http_code = 200;
            strcpy(res.http_status, "OK");
        } else if (strcmp(req.route, "/images/favicon.ico") == 0) {
            strcpy(res.route, req.route);
            strcpy(res.mime_type, "image/x-icon");
            res.http_code = 200;
            strcpy(res.http_status, "OK");
        } else if (strcmp(req.route, "/images/404.jpg") == 0) {
            strcpy(res.route, req.route);
            strcpy(res.mime_type, "image/jpg");
            res.http_code = 200;
            strcpy(res.http_status, "OK");
        } else if (strcmp(req.route, "/update") == 0) {
            sprintf(res.route, "{\"backlog\": %d, \"max_clients\": %d, \"sensor_period\": %d, \"samples_moving_average_filter\": %d}",
                this->backlog, this->max_clients, this->sensor_period, this->samples_moving_average_filter);
            strcpy(res.mime_type, "application/json; charset=utf-8");
            res.http_code = 200;
            strcpy(res.http_status, "OK");
        } else {
            res = this->not_found();
        }
        this->response(socket, res);
    }
    Signal::kill(getppid(), SIGUSR2);
}

void HttpServer::update_configuration(void) {
    FILE* fd;
    char buffer[255];
    char* key, *value;
    int changed_value = 0;
    if ( (fd = fopen(this->config_file, "r")) == NULL) {
        printf(INFO("Couldn't open the configuration file. Using default values.\n"));
        this->backlog = DEFAULT_BACKLOG;
        this->max_clients = DEFAULT_MAX_CLIENTS;
        this->sensor_period = DEFAULT_SENSOR_PERIOD;
        this->samples_moving_average_filter = DEFAULT_SAMPLES_MOVING_AVERAGE_FILTER;
        return;
    }
    while(fgets(buffer, sizeof(buffer), fd) != NULL) {
        changed_value = 0;
        buffer[strcspn(buffer, "\n")] = '\0';
        key = strtok(buffer, "=");
        value = strtok(NULL, "=");
        if (key == NULL) {
            printf(WARNING("Badly formatted key in configuration file.\n"));
            continue;
        }
        if(strcmp(key, "backlog") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_BACKLOG;
            if (changed_value == this->backlog) {
                continue;
            } else if (changed_value >= 1) {
                this->backlog = changed_value;
            }
        } else if (strcmp(key, "max_clients") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_MAX_CLIENTS;
            if (changed_value == this->max_clients) {
                continue;
            } else if (changed_value >= 1) {
                this->max_clients = changed_value;
            }
        } else if (strcmp(key, "sensor_period") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_SENSOR_PERIOD;
            if (changed_value == this->sensor_period) {
                continue;
            } else if (changed_value >= 1) {
                this->sensor_period = changed_value;
            }
        } else if (strcmp(key, "samples_moving_average_filter") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_SAMPLES_MOVING_AVERAGE_FILTER;
            if (changed_value == this->samples_moving_average_filter) {
                continue;
            } else if (changed_value >= 1) {
                this->samples_moving_average_filter = changed_value;
            }
        } else {
            printf(WARNING("Unknown key: %s.\n"), key);
            continue;
        }
        if (changed_value <= 0) {
            printf(WARNING("Invalid value for key \"%s\", old value will be kept.\n"), key);
        } else {
            printf(INFO("Configuration \"%s\" was set to %d.\n"), key, changed_value);
        }
    }
    fclose(fd);
}
