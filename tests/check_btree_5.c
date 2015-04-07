#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

START_TEST (test_5_1)
{
    chidb *db;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-5-1.dat");
    chidb_Btree_open(fname, db, &db->bt);
    test_values(db->bt, file1_keys, file1_values, file1_nvalues);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_5_2)
{
    chidb *db;
    uint16_t size;
    uint8_t *data;
    chidb_key_t nokeys[] = {0,4,6,8,9,11,18,27,36,40,100,650,1500,2500,3500,4500,5500};
    int rc;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-5-2.dat");
    chidb_Btree_open(fname, db, &db->bt);
    for(int i = 0; i<16; i++)
    {
        rc = chidb_Btree_find(db->bt, 1, nokeys[i], &data, &size);
        ck_assert(rc == CHIDB_ENOTFOUND);
    }
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


TCase* make_btree_5_tc(void)
{
    TCase *tc = tcase_create ("Step 5: Finding a value in a B-Tree");
    tcase_add_test (tc, test_5_1);
    tcase_add_test (tc, test_5_2);

    return tc;
}
