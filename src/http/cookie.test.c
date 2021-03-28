#include "unity.h"
#include "http/cookie.c"

void setUp() {
    // Nothing
}

void tearDown() {
    // Nothing
}

void test_can_define_new_cookie() {
    http_cookie_t* cookie = http_new_cookie("my_cookie", "1234",
                                            1, 200);
    TEST_ASSERT_NOT_NULL(cookie);
    TEST_ASSERT_EQUAL_STRING("my_cookie", cookie->name);
    TEST_ASSERT_EQUAL_STRING("1234",      cookie->value);
    TEST_ASSERT_EQUAL_INT(1,              cookie->max_age_def);
    TEST_ASSERT_EQUAL_UINT(200,           cookie->max_age);

    http_drop_cookie(cookie);
}

void test_can_read_completely_specified_cookie() {
    http_cookie_t* cookie = http_string_to_cookie(
        "my_cookie=1234; Max-Age=200");

    TEST_ASSERT_NOT_NULL(cookie);
    TEST_ASSERT_EQUAL_STRING("my_cookie", cookie->name);
    TEST_ASSERT_EQUAL_STRING("1234",      cookie->value);
    TEST_ASSERT_EQUAL_INT(1,              cookie->max_age_def);
    TEST_ASSERT_EQUAL_UINT(200,           cookie->max_age);

    http_drop_cookie(cookie);
}

void test_can_read_partially_specified_cookie() {
    http_cookie_t* cookie = http_string_to_cookie(
        "my_cookie=1234");

    TEST_ASSERT_NOT_NULL(cookie);
    TEST_ASSERT_EQUAL_STRING("my_cookie", cookie->name);
    TEST_ASSERT_EQUAL_STRING("1234",      cookie->value);
    TEST_ASSERT_EQUAL_INT(0,              cookie->max_age_def);

    http_drop_cookie(cookie);
}

void test_can_translate_completely_specified_cookie() {
    http_cookie_t* cookie = http_new_cookie("my_cookie", "1234",
                                            1, 200);
    char* cookie_raw      = http_cookie_to_string(cookie);

    TEST_ASSERT_NOT_NULL(cookie_raw);
    TEST_ASSERT_EQUAL_STRING("my_cookie=1234; Max-Age=200", cookie_raw);

    free(cookie_raw);
    http_drop_cookie(cookie);
}

void test_can_translate_partially_specified_cookie() {
    http_cookie_t* cookie = http_new_cookie("sessionToken", "aasf3",
                                            0, 0);
    char* cookie_raw      = http_cookie_to_string(cookie);
    
    TEST_ASSERT_NOT_NULL(cookie_raw);
    TEST_ASSERT_EQUAL_STRING("sessionToken=aasf3", cookie_raw);

    free(cookie_raw);
    http_drop_cookie(cookie);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_can_define_new_cookie);
    RUN_TEST(test_can_read_completely_specified_cookie);
    RUN_TEST(test_can_read_partially_specified_cookie);
    RUN_TEST(test_can_translate_completely_specified_cookie);
    RUN_TEST(test_can_translate_partially_specified_cookie);
    return UNITY_END();
}
