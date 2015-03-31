#include <stdlib.h>
#include <check.h>
#include "check_btree.h"


START_TEST (test_3_1)
{
    chidb *db;
    BTreeNode *btn;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-1.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);

    chidb_Btree_getNodeByPage(db->bt, 1, &btn);
    btn->type = PGTYPE_INDEX_INTERNAL;
    btn->n_cells = 2;
    btn->right_page = 3;
    btn->free_offset = 116;
    btn->cells_offset = 1008;
    chidb_Btree_writeNode(db->bt, btn);
    chidb_Btree_freeMemNode(db->bt, btn);

    chidb_Btree_getNodeByPage(db->bt, 1, &btn);
    btn_sanity_check(db->bt, btn, false);
    ck_assert(btn->type == PGTYPE_INDEX_INTERNAL);
    ck_assert(btn->n_cells == 2);
    ck_assert(btn->right_page == 3);
    ck_assert(btn->free_offset == 116);
    ck_assert(btn->cells_offset == 1008);
    chidb_Btree_freeMemNode(db->bt, btn);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_2)
{
    chidb *db;
    BTreeNode *btn;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-2.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);

    chidb_Btree_getNodeByPage(db->bt, 2, &btn);
    btn->type = PGTYPE_INDEX_LEAF;
    btn->n_cells = 3;
    btn->free_offset = 14;
    btn->cells_offset = 616;
    chidb_Btree_writeNode(db->bt, btn);
    chidb_Btree_freeMemNode(db->bt, btn);

    chidb_Btree_getNodeByPage(db->bt, 2, &btn);
    btn_sanity_check(db->bt, btn, false);
    ck_assert(btn->type == PGTYPE_INDEX_LEAF);
    ck_assert(btn->n_cells == 3);
    ck_assert(btn->free_offset == 14);
    ck_assert(btn->cells_offset == 616);
    chidb_Btree_freeMemNode(db->bt, btn);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_3)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-3.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_init_empty(db->bt, PGTYPE_TABLE_INTERNAL);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_4)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-4.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_init_empty(db->bt, PGTYPE_TABLE_LEAF);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_5)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-5.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_init_empty(db->bt, PGTYPE_INDEX_INTERNAL);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_6)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-6.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_init_empty(db->bt, PGTYPE_INDEX_LEAF);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_7)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-7.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_new_node(db->bt, PGTYPE_TABLE_INTERNAL);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_8)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-8.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_new_node(db->bt, PGTYPE_TABLE_LEAF);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_9)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-9.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_new_node(db->bt, PGTYPE_INDEX_INTERNAL);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_3_10)
{
    chidb *db;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-3-10.dat");

    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    test_new_node(db->bt, PGTYPE_INDEX_LEAF);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


TCase* make_btree_3_tc(void)
{
    TCase *tc = tcase_create ("Step 3: Creating and writing a B-Tree node to disk");
    tcase_add_test (tc, test_3_1);
    tcase_add_test (tc, test_3_2);
    tcase_add_test (tc, test_3_3);
    tcase_add_test (tc, test_3_4);
    tcase_add_test (tc, test_3_5);
    tcase_add_test (tc, test_3_6);
    tcase_add_test (tc, test_3_7);
    tcase_add_test (tc, test_3_8);
    tcase_add_test (tc, test_3_9);
    tcase_add_test (tc, test_3_10);

    return tc;
}

