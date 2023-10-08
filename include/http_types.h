#ifndef H_HTTP_TYPES
#define H_HTTP_TYPES

#define SHM_PATH    "."
#define SHM_ID      123
#define SHM_SIZE    1
#define SEM_PATH    "."
#define SEM_ID      456

#define ROUTE_SIZE 256

typedef struct serverData {
    int backlog;
    int max_clients;
    int sensor_period;
    int samples_moving_average_filter;
    int client_count;
    float temp;
} serverData;

typedef enum HttpMethod {
    GET = 0,
    POST,
    INVALID,
} HttpMethod;

typedef enum HttpConn {
    CLOSE = 0,
    KEEP_ALIVE,
} HttpConn;

typedef enum HttpCode {
    OK = 0,
    NOT_FOUND,
    BAD_REQUEST,
} HttpCode;

typedef enum HttpMimeType {
    HTML = 0,
    FAVICON,
    JPG,
    JSON,
    CSS,
    TEXT,
} HttpMimeType;

typedef struct HttpRequest {
    char route[ROUTE_SIZE];
    HttpMethod method;
} HttpRequest;

typedef struct HttpResponse {
    char route[ROUTE_SIZE];
    HttpMimeType mime_type;
    HttpCode code;
    HttpConn conn;
} HttpResponse;

extern const char http_methods[][20];
extern const char http_conns[][20];
extern const char http_codes[][20];
extern const char http_mime_types[][50];

#endif // H_HTTP_TYPES
