#include "http/http.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define BUFSIZE 8096

int __read_request(char* buffer, http_req_t* msg, http_error_t* error);
int __read_headers(char* buffer, http_header_t* headers, unsigned* num_headers, http_error_t* error); 
int __read_body(char* buffer, http_req_t* msg, http_error_t* error); 

http_req_t* http_read_request(int fd, http_error_t* error) {
    http_req_t* msg;
    uint8_t buffer[BUFSIZE + 1] = { 0 };
    unsigned readsize = 0;
    unsigned bytes_read = 0;
    int ret;

    msg = malloc(sizeof(http_req_t));
    if (msg == NULL) {
        *error = SYSTEM_ERROR;
        return NULL;
    }

    while ((bytes_read = read(fd, buffer + readsize, BUFSIZE - readsize)) > 0) {
        readsize += bytes_read;
    }

    if (bytes_read == -1) {
        *error = FD_NOT_AVAILABLE;
        http_drop_request(msg);
        return NULL;
    }

    ret = __read_request(buffer, msg, error);
    if (ret) {
        http_drop_request(msg);
        return NULL;
    }

    ret = __read_headers(buffer, msg->headers, &(msg->num_headers), error);
    if (ret) {
        http_drop_request(msg);
        return NULL;
    }

    ret = __read_body(buffer, msg, error);
    if (ret) {
        http_drop_request(msg);
        return NULL;
    }

    return msg;
    /*int regi = regcomp(
        &reg_body,
        "\r\n.*",
        REG_EXTENDED);
    if (regi) {
        // Should never get here.
    }*/
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
    int regi;

    regi = regcomp(
        &reg_request,
        "(GET|POST|HEAD|PUT|DELETE) ([[:alpha:]]+.[[:alpha:]]+.[[:alpha:]]+) (HTTP/[0-9].[0-9])\r\n",
        REG_EXTENDED);
    if (regi) {
        // Should never get here.
    }

    pmatch = malloc(4 * sizeof(regmatch_t));
    if (pmatch == NULL) {
        *error = SYSTEM_ERROR;
        regfree(&reg_request);
        return -1;
    }
    
    regi = regexec(&reg_request, buffer, 4, pmatch, 0);
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

    len = pmatch[3].rm_eo - pmatch[3].rm_so;
    start = pmatch[3].rm_so;
    msg->version = malloc((len + 1) * sizeof(char));
    if (msg->version == NULL) {
        *error = SYSTEM_ERROR;
        return -4;
    }
    strncpy(msg->version, buffer + start, len);

    return start + len + 2;
}

/* Checks how many lines in the buffer conform to the header line specification.
 * parse them and set `headers` and `num_headers` accordingly.
 */
int __read_headers(char* buffer, http_header_t* headers, unsigned* num_headers, http_error_t* error) {
    regex_t reg_header;
    regmatch_t* pmatch[MAX_HEADERS];
    int regi;

    regi = regcomp(
        &reg_header,
        "(([[:alnum:]])+: ([[:alnum:]])+\r\n)*\r\n",
        REG_EXTENDED);
    if (regi) {
        // Should never get here.
    }
    

    return 1;
}

/*
 * Searches for the body of an http reques message in `buffer` and stores a
 * copy of it in `msg`.
 */
int __read_body(char* buffer, http_req_t* msg, http_error_t* error) {
}

http_res_t* http_read_response(int fd, http_error_t* error) {
    return NULL;
}

void http_drop_request(http_req_t* msg) {
}

void http_drop_response(http_res_t* msg) {
}

