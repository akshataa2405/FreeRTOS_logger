#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int unity_test_failures;
extern int unity_test_passes;

void unity_begin(const char *suite_name);
int  unity_end(void);

#define TEST_ASSERT(cond) do { \
    if (!(cond)) { \
        printf("  [FAIL] %s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #cond); \
        unity_test_failures++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_TRUE(cond) TEST_ASSERT((cond))
#define TEST_ASSERT_FALSE(cond) TEST_ASSERT(!(cond))

#define TEST_ASSERT_EQUAL_UINT32(expected, actual) do { \
    uint32_t exp_ = (uint32_t)(expected); \
    uint32_t act_ = (uint32_t)(actual); \
    if (exp_ != act_) { \
        printf("  [FAIL] %s:%d: Expected 0x%08X was 0x%08X\n", __FILE__, __LINE__, exp_, act_); \
        unity_test_failures++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) do { \
    int exp_ = (int)(expected); \
    int act_ = (int)(actual); \
    if (exp_ != act_) { \
        printf("  [FAIL] %s:%d: Expected %d was %d\n", __FILE__, __LINE__, exp_, act_); \
        unity_test_failures++; \
        return; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    int pre_fail = unity_test_failures; \
    printf("Running %s...\n", #test_func); \
    test_func(); \
    if (unity_test_failures == pre_fail) { \
        unity_test_passes++; \
        printf("  [PASS] %s\n", #test_func); \
    } \
} while(0)

#ifdef __cplusplus
}
#endif

#endif /* UNITY_H */
