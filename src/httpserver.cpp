#include "httpserver.h"

/******************************************************************************
 * Static variables
******************************************************************************/

/// @brief Changed when SIGUSR1 is received. Tells the server to re-read the
///  configuration file.
bool HttpServer::flag_update_conf = true;

/******************************************************************************
 * Functions
******************************************************************************/

/// @brief Initializes the server
/// @param ip Server IP (0.0.0.0 by default)
/// @param port Server port (http = tcp/80 by default).
/// @param config_file Configuration file (config.cfg by default). Its content
///  are read when the signal SIGUSR1 is received.
HttpServer::HttpServer(const char* ip, const char* port, const char* config_file):
        Server(ip, port),
        shm(SHM_PATH, SHM_ID, SHM_SIZE),
        sem(SEM_PATH, SEM_ID, true) {
    char my_ip[INET6_ADDRSTRLEN];
    serverData data {
        .backlog = 0,
        .max_clients = 0,
        .sensor_period = 0,
        .samples_moving_average_filter = 0,
        .client_count = 0,
    };
    shm << data;
    strncpy(this->config_file, config_file, sizeof(this->config_file));
    Signal::set_handler(SIGUSR1, HttpServer::sigusr1_handler);
    this->get_socket().get_my_ip(my_ip);
    printf(OK("Server PID: %d.\n")
        OK("Server IP: %s\n")
        OK("Server port: %d\n"),
        getpid(), my_ip, this->get_socket().get_my_port());
}

/// @brief Before listening or accepting new connections:
///  * Check if the configuration file needs to be updated (SIGUSR1 should
///    interrupt the "accept" syscall).
void HttpServer::on_start(void) {
    if (HttpServer::flag_update_conf) {
        HttpServer::flag_update_conf = false;
        this->update_configuration();
    }
}

/// @brief This functions reads a request, and then sends a response, until
///  the socket closes.
/// @param socket The client socket.
void HttpServer::on_accept(Socket& socket) {
    HttpRequest req;
    HttpResponse res;
    while(this->request(socket, &req) == 0) {
        res = this->response_not_found();
         if (req.method == POST) {
            if (strcmp(req.route, "/dc") == 0) {
                this->sem--;
                if(this->shm[0].client_count > 0) {
                    this->shm[0].client_count--;
                }
                this->sem++;
            }
        } else if (req.method == GET) {
            if (strcmp(req.route, "/") == 0) {
                // This sleep is on purpose, to make sure that the "/dc" beacon
                // arrives first in case of refreshing the page (race condition
                // between "GET /" and "POST /dc")
                usleep(5000);
                this->sem--;
                if (this->shm[0].client_count >= this->shm[0].max_clients) {
                    res = this->response_max_clients();
                } else {
                    res = this->response_root();
                    this->shm[0].client_count++;
                }
                this->sem++;
            } else if (strcmp(req.route, "/images/favicon.ico") == 0) {
                res = this->response_favicon();
            } else if (strcmp(req.route, "/images/404.jpg") == 0) {
                res = this->response_404_image();
            } else if (strcmp(req.route, "/update") == 0) {
                res = this->response_update();
            }
        } else {
            res = this->response_bad_request();
        }
        this->response(socket, res);
    }
}

/// @brief Process a HttpRequest. Example request should be:
///     GET /route xxxx     POST /route xxxx
/// @param socket The client socket.
/// @param req Where all the request's contents will be stored.
/// @return "0" on success, "-1" on error.
int HttpServer::request(Socket& socket, HttpRequest* req) {
    char client_msg[REQUEST_SIZE];
    char *method = NULL;

    if (socket.read(client_msg, sizeof(client_msg)) > 0) {
        for(int i = 0, index_first_space = 0; i < strlen(client_msg); i++) {
            if (client_msg[i] == ' ' && index_first_space == 0) {
                index_first_space = i;
                method = (char*) malloc(index_first_space+1);
                strncpy(&method[0], client_msg, i);
                method[i] = '\0';
                if(strcmp(method, http_methods[GET]) == 0) {
                    req->method = GET;
                } else if (strcmp(method, http_methods[POST]) == 0) {
                    req->method = POST;
                } else {
                    req->method = INVALID;
                }
                free(method);
            } else if (client_msg[i] == ' ') {
                strncpy(req->route, &client_msg[index_first_space + 1], i - index_first_space - 1);
                req->route[i - index_first_space - 1] = '\0';
                break;
            }
        }
        return 0;
    }
    return -1;
}

/// @brief Generate a response and send it to the client.
/// @param socket The client socket.
/// @param res If "res.route" starts with "/", then the contents of a file
///  located inside the "SERVER_ROOT" folder will be read. Otherwise, the
///  contents of "res.route" are handled as raw data to be sended.
void HttpServer::response(Socket& socket, HttpResponse res) {
    char buffer[RESPONSE_SIZE];
    char rta[RESPONSE_SIZE];
    long bytes_read;
    long header_size;
    time_t time_var;
    if (res.route[0] == '/') {
        FILE* fd;
        strcpy(buffer, SERVER_ROOT);
        strcat(buffer, res.route);
        strcpy(res.route, buffer);
        if ((fd = fopen(res.route, "rb")) == NULL) {
            res = this->response_not_found();
        } else if ((bytes_read = fread(buffer, 1, sizeof(buffer), fd)) == 0) {
            res = this->response_not_found();
        }
        fclose(fd);
    } else {
        bytes_read = strlen(res.route);
        strcpy(buffer, res.route);
    }
    time_var = time(NULL);
    sprintf(rta,
        "HTTP/1.1 %s\n"
        "Server: %s\n"
        "Date: %s"
        "Content-Length: %ld\n"
        "Content-Type: %s\n"
        "Content-Language: en\n"
        "Connection: %s\n\n",
        http_codes[res.code],
        SERVER_NAME,
        ctime(&time_var),
        bytes_read,
        http_mime_types[res.mime_type],
        http_conns[res.conn]);
    header_size = strlen(rta);
    memcpy(&rta[header_size], buffer, bytes_read);
    socket.write(rta, header_size + bytes_read);
}

/// @brief When SIGUSR1 is received, the server will update its parameters
///  from the configuration file.
void HttpServer::sigusr1_handler(int signal) {
    HttpServer::flag_update_conf = true;
}

/// @brief Reads the configuration file and updates values.
void HttpServer::update_configuration(void) {
    FILE* fd;
    char buffer[255];
    char* key, *value;
    int changed_value = 0;

    this->sem--;
    if ( (fd = fopen(this->config_file, "r")) == NULL) {
        printf(WARNING("Couldn't open the configuration file \"%s\". Using default values.\n"), this->config_file);
        this->shm[0].backlog = DEFAULT_BACKLOG;
        this->shm[0].max_clients = DEFAULT_MAX_CLIENTS;
        this->shm[0].sensor_period = DEFAULT_SENSOR_PERIOD;
        this->shm[0].samples_moving_average_filter = DEFAULT_SAMPLES_MOVING_AVERAGE_FILTER;
        this->sem++;
        return;
    }
    while(fgets(buffer, sizeof(buffer), fd) != NULL) {
        changed_value = 0;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strstr(buffer, "=") == NULL) {
            continue;
        }
        key = strtok(buffer, "=");
        value = strtok(NULL, "=");
        if(strcmp(key, "backlog") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_BACKLOG;
            if (changed_value == this->shm[0].backlog) {
                continue;
            } else if (changed_value >= 1) {
                this->shm[0].backlog = changed_value;
            }
        } else if (strcmp(key, "max_clients") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_MAX_CLIENTS;
            if (changed_value == this->shm[0].max_clients) {
                continue;
            } else if (changed_value >= 1) {
                this->shm[0].max_clients = changed_value;
            }
        } else if (strcmp(key, "sensor_period") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_SENSOR_PERIOD;
            if (changed_value == this->shm[0].sensor_period) {
                continue;
            } else if (changed_value >= 1) {
                this->shm[0].sensor_period = changed_value;
            }
        } else if (strcmp(key, "samples_moving_average_filter") == 0) {
            changed_value = (atoi(value) != 0) ? atoi(value) : DEFAULT_SAMPLES_MOVING_AVERAGE_FILTER;
            if (changed_value == this->shm[0].samples_moving_average_filter) {
                continue;
            } else if (changed_value >= 1) {
                this->shm[0].samples_moving_average_filter = changed_value;
            }
        } else {
            printf(WARNING("Unknown key: %s.\n"), key);
            continue;
        }
        if (changed_value <= 0) {
            printf(WARNING("Invalid value for key \"%s\", old value will be kept.\n"), key);
        } else {
            printf(INFO("\"%s\" was set to %d.\n"), key, changed_value);
        }
    }
    this->sem++;
    fclose(fd);
}

/******************************************************************************
 * HTTP server responses
******************************************************************************/

HttpResponse HttpServer::response_bad_request(void) {
    HttpResponse res;
    strcpy(res.route, "/errors/bad_request.html");
    res.mime_type = HTML;
    res.code = BAD_REQUEST;
    res.conn = CLOSE;
    return res;
}

HttpResponse HttpServer::response_update(void) {
    HttpResponse res;
    sprintf(res.route,
            "{\"backlog\": %d,"
            "\"max_clients\": %d,"
            "\"sensor_period\": %d,"
            "\"samples_moving_average_filter\": %d,"
            "\"clients\": %d}",
            this->shm[0].backlog,
            this->shm[0].max_clients,
            this->shm[0].sensor_period,
            this->shm[0].samples_moving_average_filter,
            this->shm[0].client_count);
    res.mime_type = JSON;
    res.code = OK;
    res.conn = CLOSE;
    return res;
}

HttpResponse HttpServer::response_404_image(void) {
    HttpResponse res;
    strcpy(res.route, "/images/404.jpg");
    res.mime_type = JPG;
    res.code = OK;
    res.conn = CLOSE;
    return res;
}

HttpResponse HttpServer::response_root(void) {
    HttpResponse res;
    strcpy(res.route, "/index.html");
    res.mime_type = HTML;
    res.code = OK;
    res.conn = CLOSE;
    return res;
}

HttpResponse HttpServer::response_favicon(void) {
    HttpResponse res;
    strcpy(res.route, "/images/favicon.ico");
    res.mime_type = FAVICON;
    res.code = OK;
    res.conn = CLOSE;
    return res;
}

HttpResponse HttpServer::response_max_clients(void) {
    HttpResponse res;
    strcpy(res.route, "/errors/max_clients.html");
    res.mime_type = HTML;
    res.code = NOT_FOUND; // TODO, check correct code
    res.conn = CLOSE;
    return res;
}

HttpResponse HttpServer::response_not_found(void) {
    HttpResponse res;
    strcpy(res.route, "/errors/not_found.html");
    res.mime_type = HTML;
    res.code = NOT_FOUND;
    res.conn = CLOSE;
    return res;
}
