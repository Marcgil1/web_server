#include "http/cookie.h"

#include <stdlib.h>
#include <string.h>

http_cookie_t* http_new_cookie(
    char*    value,
    unsigned max_age)
{
    http_cookie_t* cookie = malloc(sizeof(http_cookie_t));
    if (cookie == NULL) {
        return NULL;
    }

    cookie->value   = strdup(value);
    cookie->max_age = max_age;

    if (cookie->value == NULL) {
        http_drop_cookie(cookie);
        return NULL;
    }

    return cookie;
}

char* http_cookie_to_string(http_cookie_t* cookie);

void http_drop_cookie(http_cookie_t* cookie) {
    free(cookie->value);
    free(cookie);
}
