#pragma once

typedef struct _http_cookie_t {
    char*    name;
    char*    value;
    int      max_age_def;
    unsigned max_age;
} http_cookie_t;

http_cookie_t* http_new_cookie(
    char*    name,
    char*    value,
    int      max_age_def,
    unsigned max_age);

char*          http_cookie_to_string(const http_cookie_t* cookie);
http_cookie_t* http_string_to_cookie(char* str);
void           http_drop_cookie(http_cookie_t* cookie);
