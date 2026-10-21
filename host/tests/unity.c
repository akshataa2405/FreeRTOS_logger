#include "unity.h"

int unity_test_failures = 0;
int unity_test_passes = 0;

void unity_begin(const char *suite_name) {
    unity_test_failures = 0;
    unity_test_passes = 0;
    printf("\n=== Running Suite: %s ===\n", suite_name);
}

int unity_end(void) {
    printf("=== Results: %d Passed, %d Failed ===\n\n", unity_test_passes, unity_test_failures);
    return unity_test_failures;
}
