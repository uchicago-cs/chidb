#include <stdlib.h>
#include <check.h>
#include "libchidb/record.h"

#define NVALUES (8)

char *str_values[] = {"foo", "bar", "foobar", "", "scrumptrulescent", "cromulent", "J.Random Hacker", "aaaaaaaaaabbbbbbbbbbaaaaaaaaaabbbbbbbbbbaaaaaaaaaabbbbbbbbbb"};
int8_t int8_values[] = {0,1,32,-32,64,-64,127,-128};
int16_t int16_values[] = {0,1,1000,-1000,20000,-20000,32767,-32768};
int32_t int32_values[] = {0,1,100000,-100000,2000000,-2000000,2147483647,-2147483648};

START_TEST (test_string)
{
    for(int i=0; i<NVALUES; i++)
    {
        DBRecord *dbr;
        char *val;
        int len;
        chidb_DBRecord_create(&dbr, "|s|", str_values[i]);
        ck_assert(dbr->nfields == 1);
        chidb_DBRecord_getString(dbr, 0, &val);
        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 0), SQL_TEXT);
        ck_assert_str_eq(str_values[i], val);
        chidb_DBRecord_getStringLength(dbr, 0, &len);
        ck_assert_int_eq(strlen(str_values[i]), len);
        chidb_DBRecord_destroy(dbr);
    }
}
END_TEST


START_TEST (test_int8)
{
    for(int i=0; i<NVALUES; i++)
    {
        DBRecord *dbr;
        int8_t val;
        chidb_DBRecord_create(&dbr, "|i1|", int8_values[i]);
        ck_assert(dbr->nfields == 1);
        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 0), SQL_INTEGER_1BYTE);
        chidb_DBRecord_getInt8(dbr, 0, &val);
        ck_assert_int_eq(int8_values[i], val);
        chidb_DBRecord_destroy(dbr);
    }
}
END_TEST


START_TEST (test_int16)
{
    for(int i=0; i<NVALUES; i++)
    {
        DBRecord *dbr;
        int16_t val;
        chidb_DBRecord_create(&dbr, "|i2|", int16_values[i]);
        ck_assert(dbr->nfields == 1);
        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 0), SQL_INTEGER_2BYTE);
        chidb_DBRecord_getInt16(dbr, 0, &val);
        ck_assert_int_eq(int16_values[i], val);
        chidb_DBRecord_destroy(dbr);
    }
}
END_TEST


START_TEST (test_int32)
{
    for(int i=0; i<NVALUES; i++)
    {
        DBRecord *dbr;
        int32_t val;
        chidb_DBRecord_create(&dbr, "|i4|", int32_values[i]);
        ck_assert(dbr->nfields == 1);
        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 0), SQL_INTEGER_4BYTE);
        chidb_DBRecord_getInt32(dbr, 0, &val);
        ck_assert_int_eq(int32_values[i], val);
        chidb_DBRecord_destroy(dbr);
    }
}
END_TEST


START_TEST (test_null)
{
    DBRecord *dbr;

    chidb_DBRecord_create(&dbr, "|0|");
    ck_assert(dbr->nfields == 1);
    ck_assert_int_eq(chidb_DBRecord_getType(dbr, 0), SQL_NULL);
    chidb_DBRecord_destroy(dbr);
}
END_TEST


START_TEST (test_multiplefields)
{
    DBRecord *dbr;
    char *s;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int len;

    for(int i=0; i<NVALUES; i++)
    {
        chidb_DBRecord_create(&dbr, "|s|0|i1|i2|i4|", str_values[i], int8_values[i], int16_values[i], int32_values[i]);
        ck_assert(dbr->nfields == 5);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 0), SQL_TEXT);
        chidb_DBRecord_getString(dbr, 0, &s);
        ck_assert_str_eq(str_values[i], s);
        chidb_DBRecord_getStringLength(dbr, 0, &len);
        ck_assert_int_eq(strlen(str_values[i]), len);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 1), SQL_NULL);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 2), SQL_INTEGER_1BYTE);
        chidb_DBRecord_getInt8(dbr, 2, &i8);
        ck_assert_int_eq(int8_values[i], i8);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 3), SQL_INTEGER_2BYTE);
        chidb_DBRecord_getInt16(dbr, 3, &i16);
        ck_assert_int_eq(int16_values[i], i16);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr, 4), SQL_INTEGER_4BYTE);
        chidb_DBRecord_getInt32(dbr, 4, &i32);
        ck_assert_int_eq(int32_values[i], i32);

        chidb_DBRecord_destroy(dbr);
    }
}
END_TEST


START_TEST (test_packunpack)
{
    DBRecord *dbr1, *dbr2;
    char *s;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    uint8_t *buf;
    int len;

    for(int i=0; i<NVALUES; i++)
    {
        chidb_DBRecord_create(&dbr1, "|s|0|i1|i2|i4|", str_values[i], int8_values[i], int16_values[i], int32_values[i]);
        chidb_DBRecord_pack(dbr1, &buf);

        chidb_DBRecord_unpack(&dbr2, buf);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr2, 0), SQL_TEXT);
        chidb_DBRecord_getString(dbr2, 0, &s);
        ck_assert_str_eq(str_values[i], s);
        chidb_DBRecord_getStringLength(dbr2, 0, &len);
        ck_assert_int_eq(strlen(str_values[i]), len);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr2, 1), SQL_NULL);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr2, 2), SQL_INTEGER_1BYTE);
        chidb_DBRecord_getInt8(dbr2, 2, &i8);
        ck_assert_int_eq(int8_values[i], i8);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr2, 3), SQL_INTEGER_2BYTE);
        chidb_DBRecord_getInt16(dbr2, 3, &i16);
        ck_assert_int_eq(int16_values[i], i16);

        ck_assert_int_eq(chidb_DBRecord_getType(dbr2, 4), SQL_INTEGER_4BYTE);
        chidb_DBRecord_getInt32(dbr2, 4, &i32);
        ck_assert_int_eq(int32_values[i], i32);

        chidb_DBRecord_destroy(dbr1);
        chidb_DBRecord_destroy(dbr2);
        free(buf);
    }
}
END_TEST


Suite* make_dbrecord_suite (void)
{
    Suite *s = suite_create ("DB Record");

    TCase *tc_single = tcase_create ("Single-value records");
    tcase_add_test (tc_single, test_string);
    tcase_add_test (tc_single, test_int8);
    tcase_add_test (tc_single, test_int16);
    tcase_add_test (tc_single, test_int32);
    tcase_add_test (tc_single, test_null);
    suite_add_tcase (s, tc_single);

    TCase *tc_multiple = tcase_create ("Multiple-field records");
    tcase_add_test (tc_multiple, test_multiplefields);
    suite_add_tcase (s, tc_multiple);

    TCase *tc_packunpack = tcase_create ("Packing/unpacking a record");
    tcase_add_test (tc_packunpack, test_packunpack);
    suite_add_tcase (s, tc_packunpack);

    return s;
}

int main (void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create (make_dbrecord_suite ());

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
