#include "unity.h"
#include "http/http.c"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void setUp() {
    // Nothing.
}

void tearDown() {
    // Nothing.
}

void test_can_read_request_with_empty_url() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "GET / HTTP/1.1\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
        &err);
    TEST_ASSERT_EQUAL_INT(16,            ret);
    TEST_ASSERT_EQUAL_INT(GET,           msg.method);
    TEST_ASSERT_EQUAL_STRING("/",        msg.url);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1", msg.version);
}

void test_can_read_request_with_file_in_url() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "POST /index.html HTTP/1.4\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
       &err);
    TEST_ASSERT_EQUAL_INT(27,               ret);
    TEST_ASSERT_EQUAL_INT(POST,             msg.method);
    TEST_ASSERT_EQUAL_STRING("/index.html", msg.url);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.4",    msg.version);
}

void test_fails_to_read_ill_formed_request() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "PASD /index.html HTTP/1.4\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
       &err);
    TEST_ASSERT_LESS_THAN_INT(0,             ret);
    TEST_ASSERT_EQUAL_INT(CORRUPTED_MESSAGE, err);
}

void test_can_read_request_with_long_url() {
    int ret;
    http_req_t msg;
    http_error_t err;

    ret = __read_request(
        "GET /very/long/path/2/my/site HTTP/1.4\r\n"
        "status: val\r\n"
        "\r\n"
        "body body body\r\n"
        "\r\n",
        &msg,
        &err);
    TEST_ASSERT_EQUAL_INT(40,                             ret);
    TEST_ASSERT_EQUAL_INT(GET,                            msg.method);
    TEST_ASSERT_EQUAL_STRING("/very/long/path/2/my/site", msg.url);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.4",                  msg.version);
}


void test_can_read_one_status_line() {
    int ret;
    http_header_t* headers;
    unsigned num_headers;
    http_error_t err;
    
    ret = __read_headers(
        "status: val\r\n"
        "\r\n"
        "body body body\r\n",
        &headers,
        &num_headers,
        &err);
    TEST_ASSERT_EQUAL_INT(15,          ret);
    TEST_ASSERT_EQUAL_UINT(1,          num_headers);
    TEST_ASSERT_EQUAL_STRING("status", headers[0].field_name);
    TEST_ASSERT_EQUAL_STRING("val",    headers[0].value);
}

void test_can_read_several_status_lines() {
    int ret;
    http_header_t* headers;
    unsigned num_headers;
    http_error_t err;

    ret = __read_headers(
        "Set-Cookie: 123\r\n"
        "Connection: close\r\n"
        "\r\n"
        "bodyyyyyy",
        &headers,
        &num_headers,
        &err);
    TEST_ASSERT_EQUAL_INT(38,              ret);
    TEST_ASSERT_EQUAL_UINT(2,              num_headers);
    TEST_ASSERT_EQUAL_STRING("Set-Cookie", headers[0].field_name);
    TEST_ASSERT_EQUAL_STRING("123",        headers[0].value);
    TEST_ASSERT_EQUAL_STRING("Connection", headers[1].field_name);
    TEST_ASSERT_EQUAL_STRING("close",      headers[1].value);
    free(headers);
}

void test_detects_error_on_ill_formed_status_line() {
    int ret;
    http_header_t* header;
    unsigned num_headers;
    http_error_t err;

    ret = __read_headers(
        "asdfalsdjfñ\r\n"
        "Set-Cookie: 123\r\n"
        "\r\n"
        "bodyyyy",
        &header,
        &num_headers,
        &err);
    TEST_ASSERT_LESS_THAN_INT(0,             ret);
    TEST_ASSERT_EQUAL_INT(CORRUPTED_MESSAGE, err);
}

void test_can_read_three_status_lines() {
    int ret;
    http_header_t* header;
    unsigned num_headers;
    http_error_t err;

    ret = __read_headers(
        "a: 1\r\n"
        "b: 2\r\n"
        "c: 3\r\n"
        "\r\n",
        &header,
        &num_headers,
        &err);
    TEST_ASSERT_EQUAL_INT(20,     ret);
    TEST_ASSERT_EQUAL_UINT(3,     num_headers);
    TEST_ASSERT_EQUAL_STRING("a", header[0].field_name);
    TEST_ASSERT_EQUAL_STRING("1", header[0].value);
    TEST_ASSERT_EQUAL_STRING("b", header[1].field_name);
    TEST_ASSERT_EQUAL_STRING("2", header[1].value);
    TEST_ASSERT_EQUAL_STRING("c", header[2].field_name);
    TEST_ASSERT_EQUAL_STRING("3", header[2].value);
}

void test_can_read_complete_request() {
    int fd;
    http_req_t* msg;
    http_error_t err;

    fd = open("resources/test/http/http1.txt", O_RDONLY);
    if (fd == -1) {
        TEST_FAIL();
    }

    msg = http_read_request(fd, &err);
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_EQUAL_INT(GET,               msg->method);
    TEST_ASSERT_EQUAL_STRING("/",            msg->url);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1",     msg->version);
    TEST_ASSERT_EQUAL_UINT(1,                msg->num_headers);
    TEST_ASSERT_EQUAL_STRING("Host",         msg->headers[0].field_name);
    TEST_ASSERT_EQUAL_STRING("www.sstt.org", msg->headers[0].value);
    
    http_drop_request(msg);

    close(fd);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_can_read_request_with_empty_url);
    RUN_TEST(test_can_read_request_with_file_in_url);
    RUN_TEST(test_fails_to_read_ill_formed_request);
    RUN_TEST(test_can_read_request_with_long_url);
    RUN_TEST(test_can_read_one_status_line);
    RUN_TEST(test_can_read_several_status_lines);
    RUN_TEST(test_detects_error_on_ill_formed_status_line);
    RUN_TEST(test_can_read_three_status_lines);
    RUN_TEST(test_can_read_complete_request);
    return UNITY_END();
}
