#include "http/cookie.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

http_cookie_t* http_new_cookie(
    char*    name,
    char*    value,
    int      max_age_def,
    unsigned max_age)
{
    if (name == NULL || value == NULL) {
        return NULL;
    }

    http_cookie_t* cookie = malloc(sizeof(http_cookie_t));
    if (cookie == NULL) {
        return NULL;
    }

    cookie->name        = strdup(name);
    cookie->value       = strdup(value);
    cookie->max_age_def = max_age_def;
    cookie->max_age     = max_age;

    if (cookie->name == NULL || cookie->value == NULL) {
        http_drop_cookie(cookie);
        return NULL;
    }

    return cookie;
}

char* http_cookie_to_string(const http_cookie_t* cookie) {
    if (cookie == NULL) {
        return NULL;
    }
    char*  raw;
    size_t raw_len = strlen(cookie->name) + 1 + strlen(cookie->value);
    if (cookie->max_age_def) {
        raw_len += 2 + strlen("Max-Age") + (cookie->max_age == 0
                        ? 1
                        : ((size_t)(log10(cookie->max_age) + 1)));
        raw = malloc((raw_len + 1) * sizeof(char));
        if (raw == NULL) {
            return NULL;
        }
        sprintf(raw, "%s=%s; Max-Age=%u",
                cookie->name, cookie->value, cookie->max_age);
    } else {
        raw_len += 0;
        raw = malloc((raw_len + 1) * sizeof(char));
        if (raw == NULL) {
            return NULL;
        }
        sprintf(raw, "%s=%s",
                cookie->name, cookie->value);
    }
    return raw;
}

http_cookie_t* http_string_to_cookie(char* str) {
    if (str == NULL) {
        return NULL;
    }

    http_cookie_t* cookie = malloc(sizeof(http_cookie_t));
    if (cookie == NULL) {
        return NULL;
    }

    char* str_dup = strdup(str);
    if (str_dup == NULL) {
        return NULL;
    }

    cookie->name        = strdup(strtok(str_dup, "="));
    cookie->value       = strdup(strtok(NULL, ";"));
    cookie->max_age_def = 0;

    char* attribute;
    char* attribute_val;
    char* ptr;
    while ((ptr = strtok(NULL, "=")) != NULL) {
        attribute     = strdup(ptr + 1);
        attribute_val = strdup(strtok(NULL, ";"));

        if (strcmp(attribute, "Max-Age") == 0) {
            cookie->max_age_def = 1;
            cookie->max_age     = (unsigned) strtol(attribute_val, NULL, 10);
            free(attribute_val);
        } else {
            free(attribute_val);
        }

        free(attribute);
    }
    
    free(str_dup);

    return cookie;
}
void http_drop_cookie(http_cookie_t* cookie) {
    if (cookie == NULL) {
        return;
    }

    free(cookie->name);
    free(cookie->value);
    free(cookie);
}
