#include "http_types.h"

const char http_methods[][20] = {
    "GET",
    "POST",
};

const char http_conns[][20] = {
    "close",
    "keep-alive",
};

const char http_codes[][20] = {
    "200 OK",
    "404 Not Found",
    "400 Bad Request",
};

const char http_mime_types[][50] = {
    "text/html; charset=utf-8",
    "image/x-icon",
    "image/jpg",
    "application/json; charset=utf-8",
    "text/css; charset=utf-8",
    "text/plain; charset=utf-8",
};