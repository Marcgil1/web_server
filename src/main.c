#include <stdio.h>
#include <regex.h>

#include "http/http.h"


int main() {
    regex_t url;
    int ret;

    regcomp(
        &url,
        "[[:alpha:]]+\.[[:alpha:]]+\.[[:alpha:]]+",
        REG_EXTENDED);
    ret = regexec(
        &url,
        "www.web.com",
        0,
        NULL,
        0);
    regfree(&url);
    printf("%d\n", ret);


    return 0;
}

