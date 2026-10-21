#include "unity.h"
#include "record_format.h"
#include <string.h>

void test_struct_sizes(void) {
    TEST_ASSERT_EQUAL_INT(28, (int)sizeof(DataRecord_t));
    TEST_ASSERT_EQUAL_INT(512, (int)sizeof(LogBlock_t));
}

void test_record_packing_roundtrip(void) {
    DataRecord_t rec;
    memset(&rec, 0, sizeof(rec));

    rec.seq = 42;
    rec.timestamp_us = 1234567;
    rec.adc_mean = 2048;
    rec.adc_min = 2000;
    rec.adc_max = 2100;
    rec.ax = -500;
    rec.ay = 1000;
    rec.az = 16384;
    rec.gx = 10;
    rec.gy = -20;
    rec.gz = 5;
    rec.flags = RECORD_FLAG_NORMAL;

    uint8_t buffer[28];
    memcpy(buffer, &rec, sizeof(rec));

    DataRecord_t unpacked;
    memcpy(&unpacked, buffer, sizeof(unpacked));

    TEST_ASSERT_EQUAL_UINT32(42, unpacked.seq);
    TEST_ASSERT_EQUAL_UINT32(1234567, unpacked.timestamp_us);
    TEST_ASSERT_EQUAL_INT(2048, unpacked.adc_mean);
    TEST_ASSERT_EQUAL_INT(16384, unpacked.az);
}
