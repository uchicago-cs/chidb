#include <stdlib.h>
#include <check.h>
#include "libchidb/util.h"

#define NVALUES (8)

uint16_t uint16_values[] = {0,1,128,255,256,32767,32768,65535};
uint32_t uint32_values[] = {0,255,256,32767,32768,65535,65536,4294967295};
uint32_t varint32_values[] = {0,255,256,32767,32768,65535,65536,268435455};

START_TEST (test_getput2byte)
{
    uint8_t buf[2];

    for(int i=0; i<NVALUES; i++)
    {
        uint16_t val;
        put2byte(buf, uint16_values[i]);
        val = get2byte(buf);

        ck_assert_int_eq(val, uint16_values[i]);
    }
}
END_TEST


START_TEST (test_getput4byte)
{
    uint8_t buf[4];

    for(int i=0; i<NVALUES; i++)
    {
        uint32_t val;
        put4byte(buf, uint32_values[i]);
        val = get4byte(buf);

        ck_assert_int_eq(val, uint32_values[i]);
    }
}
END_TEST


START_TEST (test_varint32)
{
    uint8_t buf[4];

    for(int i=0; i<NVALUES; i++)
    {
        uint32_t val;
        putVarint32(buf, varint32_values[i]);
        getVarint32(buf, &val);

        ck_assert_int_eq(val, varint32_values[i]);
    }
}
END_TEST


Suite* make_utils_suite (void)
{
    Suite *s = suite_create ("Utils");

    TCase *tc_integer = tcase_create ("Integer manipulation functions");
    tcase_add_test (tc_integer, test_getput2byte);
    tcase_add_test (tc_integer, test_getput4byte);
    tcase_add_test (tc_integer, test_varint32);
    suite_add_tcase (s, tc_integer);

    return s;
}

int main (void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create (make_utils_suite ());

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
