#ifndef H_HTTP_TYPES
#define H_HTTP_TYPES

typedef enum HttpMethod {
    GET = 0,
    POST,
} HttpMethod;

typedef enum HttpConn {
    CLOSE = 0,
    KEEP_ALIVE,
} HttpConn;

typedef enum HttpCode {
    OK = 0,
    NOT_FOUND,
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
    char route[256];
    HttpMethod method;
} HttpRequest;

typedef struct HttpResponse {
    char route[256];
    HttpMimeType mime_type;
    HttpCode code;
    HttpConn conn;
} HttpResponse;

extern const char http_methods[][20];
extern const char http_conns[][20];
extern const char http_codes[][20];
extern const char http_mime_types[][50];

#endif // H_HTTP_TYPES
