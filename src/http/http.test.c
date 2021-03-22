#include "unity.h"
#include "http/http.c"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void setUp() {
    // Nothing.
}

void tearDown() {
    // Nothing.
}

void test_can_read_request1() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "GET website.h.com HTTP/1.1\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
        &err);
    TEST_ASSERT_EQUAL_INT(28,                   ret);
    TEST_ASSERT_EQUAL_INT(GET,                  msg.method);
    TEST_ASSERT_EQUAL_STRING("website.h.com",   msg.url);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1",        msg.version);
}

void test_can_read_request2() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "POST website.hff.com HTTP/1.4\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
       &err);
    TEST_ASSERT_EQUAL_INT(31,                   ret);
    TEST_ASSERT_EQUAL_INT(POST,                 msg.method);
    TEST_ASSERT_EQUAL_STRING("website.hff.com", msg.url);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.4",        msg.version);
}

void test_can_read_request3() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "PASD website.hff.com HTTP/1.4\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
       &err);
    TEST_ASSERT_LESS_THAN_INT(0,             ret);
    TEST_ASSERT_EQUAL_INT(CORRUPTED_MESSAGE, err);
}

void test_can_read_status_lines1() {
    TEST_ASSERT_TRUE(1);
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_can_read_request1);
    RUN_TEST(test_can_read_request2);
    RUN_TEST(test_can_read_request3);
    return UNITY_END();
}
