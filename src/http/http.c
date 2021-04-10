#include "http/http.h"
#include "dg/debug.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define BUFSIZE 8096

int __read_request(char* buffer, http_req_t* msg, http_error_t* error);
int __read_headers(char* buffer, http_header_t** headers, unsigned* num_headers, int fd, http_error_t* error); 
int __read_body(char* buffer, http_req_t* msg, http_error_t* error); 

http_req_t* http_read_request(int fd, http_error_t* error) {
    http_req_t* msg;
    uint8_t buffer[BUFSIZE + 1] = { 0 };
    unsigned readsize = 0;
    unsigned bytes_read = 0;
    int start;
    int ret;

    msg = malloc(sizeof(http_req_t));
    if (msg == NULL) {
        *error = SYSTEM_ERROR;
        return NULL;
    }
    msg->method = 0;
    msg->url = NULL;
    msg->version = NULL;
    msg->num_headers = 0;
    msg->headers = NULL;
    msg->body = 0;

    bytes_read = read(fd, buffer, BUFSIZE);

    if (bytes_read == -1) {
        *error = FD_NOT_AVAILABLE;
        http_drop_request(msg);
        return NULL;
    }

    start = 0;
    ret = __read_request(buffer, msg, error);
    if (ret < 0) {
        http_drop_request(msg);
        return NULL;
    }

    start += ret;
    ret = __read_headers(buffer + start, &(msg->headers), &(msg->num_headers), fd, error);
    if (ret < 0) {
        http_drop_request(msg);
        return NULL;
    }

    start += ret;
    ret = __read_body(buffer + start, msg, error);
    if (ret < 0) {
        http_drop_request(msg);
        return NULL;
    }

    return msg;
}

/*
 * Checks whether the first line of `buffer` is a valid request line.
 * If successful, fills the `method`, `url` and `version` fields of `msg` and
 * returns 0. Else specifies an error code and returns 1.
 */
int __read_request(char* buffer, http_req_t* msg, http_error_t* error) {
    regex_t reg_request;
    regmatch_t* pmatch;
    unsigned start_idx;
    unsigned end_idx;
    unsigned len;
    unsigned start;
    unsigned num_groups;
    int regi;

    num_groups = 5;
    regi = regcomp(
        &reg_request,
        "(GET|POST|HEAD|PUT|DELETE) (/|(/([[:alpha:]]|[[:alpha:]]\.)+)+) (HTTP/1\.1)\r\n",
        REG_EXTENDED);
    if (regi) {
        // Should never get here.
    }

    pmatch = malloc((num_groups+1) * sizeof(regmatch_t));
    if (pmatch == NULL) {
        *error = SYSTEM_ERROR;
        regfree(&reg_request);
        return -1;
    }
    
    regi = regexec(&reg_request, buffer, num_groups+1, pmatch, 0);
    if (regi || pmatch[0].rm_so != 0) {
        *error = CORRUPTED_MESSAGE;
        regfree(&reg_request);
        return -2;
    } else {
        regfree(&reg_request);
    }

    // The first line is now known to conform to the HTTP header regex.
    // Determine the request type according to the message's first chars.
    switch (buffer[0]) {
        case 'G':
            msg->method = GET;
            break;
        case 'P':
            switch (buffer[1]) {
                case 'O':
                    msg->method = POST;
                    break;
                case 'U':
                    msg->method = PUT;
                    break;
                default:
                    // Should never get here.
                    break;
            }
            break;
        case 'H':
            msg->method = HEAD;
            break;
        case 'D':
            msg->method = DELETE;
            break;
        default:
            // Should never get here.
            break;
    }
    
    // Including the spot for \0 at the end.
    len = pmatch[2].rm_eo - pmatch[2].rm_so;
    start = pmatch[2].rm_so;
    msg->url = malloc((len + 1) * sizeof(char));
    if (msg->url == NULL) {
        *error = SYSTEM_ERROR;
        return -3;
    }
    strncpy(msg->url, buffer + start, len);
    msg->url[len] = '\0';

    len = pmatch[5].rm_eo - pmatch[5].rm_so;
    start = pmatch[5].rm_so;
    msg->version = malloc((len + 1) * sizeof(char));
    if (msg->version == NULL) {
        *error = SYSTEM_ERROR;
        return -4;
    }
    strncpy(msg->version, buffer + start, len);
    msg->version[len] = '\0';

    return start + len + 2;
}

/* Checks how many lines in the buffer conform to the header line specification.
 * parse them and set `headers` and `num_headers` accordingly.
 */
int __read_headers(char* buffer, http_header_t** headers, unsigned* num_headers, int fd, http_error_t* err) {
    regex_t reg_header;
    unsigned num_groups = 4;
    regmatch_t pmatch[num_groups+1];
    unsigned top;
    unsigned idx;
    unsigned len;
    unsigned start;
    int regi;

    *headers = malloc(MAX_HEADERS * sizeof(http_header_t));
    if (*headers == NULL) {
        *err = SYSTEM_ERROR;
        return -1;
    }

    regi = regcomp(
        &reg_header,
        "^(([[:alnum:]]|-|_)+): (([[:print:]])+)\r\n",
        REG_EXTENDED);
    if (regi) {
        // Should never get here.
    }

    if (strlen(buffer) == 0) {
        read(fd, buffer, BUFSIZE);
    }

    idx = 0;
    top = 0;
    while ((regi = regexec(&reg_header, buffer + top, num_groups+1, pmatch, 0))
           != REG_NOMATCH)
    {
        len = pmatch[1].rm_eo - pmatch[1].rm_so;
        start = pmatch[1].rm_so;
        (*headers)[idx].field_name = malloc((len + 1) * sizeof(char));
        if ((*headers)[idx].field_name == NULL) {
            *err = SYSTEM_ERROR;
            return -2;
        }
        strncpy((*headers)[idx].field_name, buffer + top + start, len);
        (*headers)[idx].field_name[len] = '\0';
        
        len = pmatch[3].rm_eo - pmatch[3].rm_so;
        start = pmatch[3].rm_so;
        (*headers)[idx].value = malloc((len + 1) * sizeof(char));
        if ((*headers)[idx].value == NULL) {
            *err = SYSTEM_ERROR;
            return -3;
        }
        strncpy((*headers)[idx].value, buffer + top + start, len);
        (*headers)[idx].value[len] = '\0';

        idx++;
        top += pmatch[0].rm_eo;

        if (strlen(buffer + top) == 0) {
            read(fd, buffer + top, BUFSIZE - top);
        }

   
        
        if (idx == MAX_HEADERS) {
            *err = CORRUPTED_MESSAGE;
            return -4;
        }
    }

    if (strlen(buffer + top) == 0) {
        read(fd, buffer + top, BUFSIZE - top);
    }

    *num_headers = idx;
    if (strncmp(buffer + top, "\r\n", 2) != 0) {
        *err = CORRUPTED_MESSAGE;
        return -5;
    }

    if (realloc(*headers, *num_headers * sizeof(http_header_t)) == NULL) {
        *err = SYSTEM_ERROR;
        return -6;
    }

    return top + 2;
}

/*
 * Searches for the body of an http reques message in `buffer` and stores a
 * copy of it in `msg`.
 */
int __read_body(char* buffer, http_req_t* msg, http_error_t* err) {
    int len;
    
    len = strlen(buffer);

    msg->body = malloc((len+1) * sizeof(char));
    if (msg->body == NULL) {
        *err = SYSTEM_ERROR;
        return -1;
    }

    strcpy(msg->body, buffer);
    msg->body[len] = '\0';

    return len;
}

http_res_t* http_read_response(int fd, http_error_t* error) {
    return NULL;
}
int http_write_request(int fd, http_req_t* msg, http_error_t* error) {
    return 0;
}

int http_write_response(int fd, http_res_t* msg, http_error_t* error) {
    char* raw;
    size_t len;
    size_t aux;
    size_t written;

    raw = http_response_to_string(msg);
    if (raw == NULL) {
        *error = SYSTEM_ERROR;
        return 1;
    }

    len = strlen(raw);
    send(fd, raw, len, 0);
/*    written = 0;
    while (written != len) {
        aux = write(fd, raw + written, len - written);

        if (aux == -1) {
            *error = SYSTEM_ERROR;
            return 1;
        } else {
            written += aux;
        }
    }

    return 0;*/
}

http_header_t* http_new_headers(
    unsigned num_headers, ...)
{
    http_header_t* headers;
    char* string;
    size_t len;
    va_list ap;
    size_t idx;

    headers = malloc(MAX_HEADERS * sizeof(http_header_t));
    if (headers == NULL) {
        return NULL;
    }
    
    va_start(ap, num_headers);
    for (idx = 0; idx < num_headers; idx++) {
        string                  = va_arg(ap, char*);
        len                     = strlen(string) + 1;
        headers[idx].field_name = malloc(len * sizeof(char));
        if (headers[idx].field_name == NULL) {
            return NULL;
        }
        strcpy(headers[idx].field_name, string);

        string             = va_arg(ap, char*);
        len                = strlen(string) + 1;
        headers[idx].value = malloc(len * sizeof(char));
        if (headers[idx].field_name == NULL) {
            return NULL;
        }
        strcpy(headers[idx].value, string);
    }
    va_end(ap);

    return headers;
}

http_req_t* http_new_request(
    http_method_t  method,
    char*          url,
    char*          version,
    unsigned       num_headers,
    http_header_t* headers,
    char*          body)
{
    http_req_t* msg;

    if (url == NULL || version == NULL || headers == NULL || body == NULL) {
        return NULL;
    }

    msg = malloc(sizeof(http_req_t));
    if (msg == NULL) {
        return NULL;
    }

    msg->method      = method;
    msg->url         = strdup(url);
    msg->version     = strdup(version);
    msg->num_headers = num_headers;
    msg->body        = strdup(body);

    return msg;
}

http_res_t* http_new_response(
    char*          version,
    unsigned       status_code,
    char*          phrase,
    unsigned       num_headers,
    http_header_t* headers,
    char*          body)
{
    http_res_t* msg;

    if (version == NULL || phrase == NULL || headers == NULL || body == NULL) {
        return NULL;
    }

    msg = malloc(sizeof(http_res_t));
    if (msg == NULL) {
        return NULL;
    }

    msg->version     = strdup(version);
    msg->status_code = status_code;
    msg->phrase      = strdup(phrase);
    msg->num_headers = num_headers;
    msg->headers     = headers;
    msg->body        = strdup(body);

    return msg;
}

void __http_drop_headers(unsigned num_headers, http_header_t* headers) {
    if (headers == NULL) {
        return;
    }

    for (int i = 0; i < num_headers; i++) {
        free(headers[i].field_name);
        free(headers[i].value);
    }
    free(headers);
}

size_t __get_raw_response_size(http_res_t* msg) {
    size_t size;
    unsigned status_code;

    status_code = msg->status_code;
    size = 0;
    size += strlen(msg->version);
    size += 1; // Space.
    while (status_code > 0) {
        status_code /= 10;
        size += 1;
    }
    size += 1; // Space.
    size += strlen(msg->phrase);
    size += 2; // '\r\n'
    for (int i = 0; i < msg->num_headers; i++) {
        size += strlen(msg->headers[i].field_name);
        size += 2; // ': '
        size += strlen(msg->headers[i].value);
        size += 2; // '\r\n'
    }
    size += 2; // '\r\n'
    size += strlen(msg->body);
    size += 1; // '\0'

    return size;
}

char* http_response_to_string(http_res_t* msg) {
    size_t buff_len = 128;
    char buff[buff_len];
    char* raw;
    size_t len;
    size_t idx;

    if (msg == NULL) {
        return NULL;
    }

    len = __get_raw_response_size(msg);
    
    raw = malloc(len * sizeof(char));
    if (raw == NULL) {
        return NULL;
    }
    memset(raw, 0, len);
    memset(buff, 0, buff_len);

    strcat(raw, msg->version);
    strcat(raw, " ");
    snprintf(buff, buff_len, "%u", msg->status_code);
    strcat(raw, buff);
    strcat(raw, " ");
    strcat(raw, msg->phrase);
    strcat(raw, "\r\n");
    for (int i = 0; i < msg->num_headers; i++) {
        strcat(raw, msg->headers[i].field_name);
        strcat(raw, ": ");
        strcat(raw, msg->headers[i].value);
        strcat(raw, "\r\n");
    }
    strcat(raw, "\r\n");
    strcat(raw, msg->body);

    return raw;
}

char* http_request_to_string(http_req_t* msg) {
    return NULL;
}

http_header_t* http_new_header(char* field_name, char* value) {
    if (field_name == NULL || value == NULL)
        return NULL;

    http_header_t* ans = malloc(sizeof(http_header_t));
    if (ans == NULL)
        return NULL;

    ans->field_name = strdup(field_name);
    ans->value      = strdup(value);
    if (ans->field_name == NULL || ans->value == NULL) {
        http_drop_header(ans);
        return NULL;
    } else {
        return ans;
    }
}

void http_drop_header(http_header_t* header) {
    if (header == NULL)
        return;

    free(header->field_name);
    free(header->value);
    free(header);
}

http_header_t* http_get_header(http_req_t* msg, char* header) {
    if (msg == NULL)
        return NULL;

    for (size_t i = 0; i < msg->num_headers; i++)
        if (strcmp(msg->headers[i].field_name, header) == 0)
            return http_new_header(header, msg->headers[i].value);

    return NULL;
}

http_cookie_t* http_get_cookie(http_req_t* msg, char* cookie) {
    if (msg == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < msg->num_headers; i++) {
        char* idx;
        if (strcmp(msg->headers[i].field_name, "Cookie") != 0)
            continue;
        if (strstr(msg->headers[i].value, cookie) != msg->headers[i].value)
            continue;
        if (msg->headers[i].value + strlen(cookie) != (idx = strchr(msg->headers[i].value, '=')))
            continue;
        return http_string_to_cookie(msg->headers[i].value);
    }

    return NULL;
}


void http_drop_request(http_req_t* msg) {
    if (msg == NULL) {
        return;
    }
    
    free(msg->url);
    free(msg->version);
    __http_drop_headers(msg->num_headers, msg->headers);
    free(msg->body);
    free(msg);
}

void http_drop_response(http_res_t* msg) {
    if (msg == NULL) {
        return;
    }

    free(msg->version);
    free(msg->phrase);
    __http_drop_headers(msg->num_headers, msg->headers);
    free(msg->body);
    free(msg);
}

