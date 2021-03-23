#pragma once

/* Maximum number of headers to be read */
#define MAX_HEADERS 32

typedef struct _http_header_t {
    char* field_name;
    char* value;
} http_header_t;

typedef enum _http_method_t {
    GET,
    POST,
    HEAD,
    PUT,
    DELETE,
} http_method_t;

typedef struct _http_req_t {
    http_method_t  method;
    char*          url;
    char*          version;
    unsigned       num_headers;
    http_header_t* headers;
    char*          body;
} http_req_t;

typedef struct _http_res_t {
    char*          version;
    unsigned       status_code;
    char*          phrase;
    unsigned       num_headers;
    http_header_t* headers;
    char*          body;
} http_res_t;

typedef enum _http_error_t {
    FD_NOT_AVAILABLE,
    CORRUPTED_MESSAGE,
    SYSTEM_ERROR,
} http_error_t;

http_req_t* http_read_request(int fd, http_error_t* error);
http_res_t* http_read_response(int fd, http_error_t* error);

int http_write_request(int fd, http_req_t* msg, http_error_t* error);
int http_write_response(int fd, http_res_t* msg, http_error_t* error);

http_header_t* http_new_headers(
    unsigned num_headers, ...);
http_req_t* http_new_request(
    http_method_t  method,
    char*          url,
    char*          version,
    unsigned       num_headers,
    http_header_t* headers,
    char*          body);
http_res_t* http_new_response(
    char*          version,
    unsigned       status_code,
    char*          phrase,
    unsigned       num_headers,
    http_header_t* headers,
    char*          body);

char* http_request_to_string(http_req_t* msg);
char* http_response_to_string(http_res_t* msg);

void http_drop_request(http_req_t* msg);
void http_drop_response(http_res_t* msg);

