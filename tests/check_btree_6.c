#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

START_TEST (test_6_1)
{
    chidb *db;
    int rc;
    npage_t pages[] = {4,3,2};
    chidb_key_t keys[] = {4,9,6000};
    char *values[] = {"foo4","foo9","foo6000"};


    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-6-1.dat");
    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);

    for(int i=0; i<3; i++)
    {
        rc = chidb_Btree_insertInTable(db->bt, pages[i], keys[i], (uint8_t *) values[i], 128);
        ck_assert(rc == CHIDB_OK);
    }

    test_values(db->bt, file1_keys, file1_values, file1_nvalues);
    test_values(db->bt, keys, values, 3);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_6_2)
{
    chidb *db;
    int rc;
    chidb_key_t keys[] = {4,9,6000};
    char *values[] = {"foo4","foo9","foo6000"};


    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-6-2.dat");
    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);

    for(int i=0; i<3; i++)
    {
        rc = chidb_Btree_insertInTable(db->bt, 1, keys[i], (uint8_t *) values[i], 128);
        ck_assert(rc == CHIDB_OK);
    }

    test_values(db->bt, file1_keys, file1_values, file1_nvalues);
    test_values(db->bt, keys, values, 3);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST

TCase* make_btree_6_tc(void)
{
    TCase *tc = tcase_create ("Step 6: Insertion into a leaf without splitting");
    tcase_add_test (tc, test_6_1);
    tcase_add_test (tc, test_6_2);

    return tc;
}

