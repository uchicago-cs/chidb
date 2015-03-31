#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

START_TEST (test_7_1)
{
    chidb *db;
    int rc;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    for(int i=0; i<bigfile_nvalues; i++)
        insert_bigfile(db, i);

    test_bigfile(db);

    chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


START_TEST (test_7_2)
{
    chidb *db;
    int rc;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    for(int i=bigfile_nvalues-1; i>=0; i--)
        insert_bigfile(db, i);

    test_bigfile(db);

    chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


START_TEST (test_7_3)
{
    chidb *db;
    int rc;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    for(int i=0; i<bigfile_nvalues; i+=2)
        insert_bigfile(db, i);
    for(int i=1; i<bigfile_nvalues; i+=2)
        insert_bigfile(db, i);

    test_bigfile(db);

    chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


TCase* make_btree_7_tc(void)
{
    TCase *tc = tcase_create ("Step 7: Insertion with splitting");
    tcase_add_test (tc, test_7_1);
    tcase_add_test (tc, test_7_2);
    tcase_add_test (tc, test_7_3);

    return tc;
}

