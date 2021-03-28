#pragma once

typedef struct _http_cookie_t {
    char*    value;
    unsigned max_age;
} http_cookie_t;

http_cookie_t* http_new_cookie(
    char*    value,
    unsigned max_age);

char* http_cookie_to_string(http_cookie_t* cookie);

void http_drop_cookie(http_cookie_t* cookie);
