#include <stdlib.h>
#include <check.h>
#include "check_btree.h"


START_TEST (test_1a_1)
{
    int rc;
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-1a-1.dat");

    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);
    ck_assert(db->bt->pager->n_pages == 5);
    ck_assert(db->bt->pager->page_size == 1024);
    rc = chidb_Btree_close(db->bt);
    ck_assert(rc == CHIDB_OK);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_1a_2)
{
    int rc;
    chidb *db;

    db = malloc(sizeof(chidb));

    char *fname = create_copy(TESTFILE_CORRUPT1, "btree-test-1a-2.dat");
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_ECORRUPTHEADER);
    delete_copy(fname);

    free(db);
}
END_TEST

START_TEST (test_1a_3)
{
    int rc;
    chidb *db;

    db = malloc(sizeof(chidb));

    char *fname = create_copy(TESTFILE_CORRUPT2, "btree-test-1a-3.dat");
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_ECORRUPTHEADER);
    delete_copy(fname);

    free(db);
}
END_TEST

START_TEST (test_1a_4)
{
    int rc;
    chidb *db;

    db = malloc(sizeof(chidb));

    char *fname = create_copy(TESTFILE_CORRUPT3, "btree-test-1a-4.dat");
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_ECORRUPTHEADER);
    delete_copy(fname);

    free(db);
}
END_TEST


START_TEST (test_1a_5)
{
    int rc;
    chidb *db;

    db = malloc(sizeof(chidb));

    char *fname = create_copy(TESTFILE_STRINGS2, "btree-test-1a-5.dat");
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);
    ck_assert(db->bt->pager->n_pages == 7);
    ck_assert(db->bt->pager->page_size == 1024);
    rc = chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


TCase* make_btree_1a_tc(void)
{
    TCase *tc = tcase_create ("Step 1a: Opening an existing chidb file");
    tcase_add_test (tc, test_1a_1);
    tcase_add_test (tc, test_1a_2);
    tcase_add_test (tc, test_1a_3);
    tcase_add_test (tc, test_1a_4);
    tcase_add_test (tc, test_1a_5);

    return tc;
}
