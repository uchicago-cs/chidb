#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

START_TEST (test_8_1)
{
    chidb *db;
    int rc;
    npage_t npage;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    for(int i=0; i<bigfile_nvalues; i++)
        insert_bigfile(db, i);

    chidb_Btree_newNode(db->bt, &npage, PGTYPE_INDEX_LEAF);
    for(int i=0; i<bigfile_nvalues; i++)
        chidb_Btree_insertInIndex(db->bt, npage, bigfile_ikeys[i], bigfile_pkeys[i]);

    test_index_bigfile(db, npage);

    chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


START_TEST (test_8_2)
{
    chidb *db;
    int rc;
    npage_t npage;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    for(int i=0; i<bigfile_nvalues; i++)
        insert_bigfile(db, i);

    chidb_Btree_newNode(db->bt, &npage, PGTYPE_INDEX_LEAF);
    for(int i=bigfile_nvalues-1; i>=0; i--)
        chidb_Btree_insertInIndex(db->bt, npage, bigfile_ikeys[i], bigfile_pkeys[i]);

    test_index_bigfile(db, npage);

    chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


START_TEST (test_8_3)
{
    chidb *db;
    int rc;
    npage_t npage;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    for(int i=0; i<bigfile_nvalues; i++)
        insert_bigfile(db, i);

    chidb_Btree_newNode(db->bt, &npage, PGTYPE_INDEX_LEAF);
    for(int i=0; i<bigfile_nvalues; i+=2)
        chidb_Btree_insertInIndex(db->bt, npage, bigfile_ikeys[i], bigfile_pkeys[i]);
    for(int i=1; i<bigfile_nvalues; i+=2)
        chidb_Btree_insertInIndex(db->bt, npage, bigfile_ikeys[i], bigfile_pkeys[i]);

    test_index_bigfile(db, npage);

    chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


TCase* make_btree_8_tc(void)
{
    TCase *tc = tcase_create ("Step 8: Supporting index B-Trees");
    tcase_add_test (tc, test_8_1);
    tcase_add_test (tc, test_8_2);
    tcase_add_test (tc, test_8_3);

    return tc;
}

