#include <stdlib.h>
#include <check.h>
#include "check_btree.h"


START_TEST (test_2_1)
{
    int rc;
    chidb *db;
    BTreeNode *btn;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-2-1.dat");
    chidb_Btree_open(fname, db, &db->bt);

    rc = chidb_Btree_getNodeByPage(db->bt, 1, &btn);
    ck_assert(rc == CHIDB_OK);

    rc = chidb_Btree_freeMemNode(db->bt, btn);
    ck_assert(rc == CHIDB_OK);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_2_2)
{
    chidb *db;
    BTreeNode *btn;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-2-2.dat");
    chidb_Btree_open(fname, db, &db->bt);
    chidb_Btree_getNodeByPage(db->bt, 1, &btn);

    btn_sanity_check(db->bt, btn, false);
    ck_assert(btn->page->npage == 1);
    ck_assert(btn->type == PGTYPE_TABLE_INTERNAL);
    ck_assert(btn->n_cells == 3);
    ck_assert(btn->right_page == 2);
    ck_assert(btn->free_offset == 118);
    ck_assert(btn->cells_offset == 1000);
    ck_assert(get2byte(&btn->celloffset_array[0]) == 1008);
    ck_assert(get2byte(&btn->celloffset_array[2]) == 1016);
    ck_assert(get2byte(&btn->celloffset_array[4]) == 1000);

    chidb_Btree_freeMemNode(db->bt, btn);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_2_3)
{
    chidb *db;
    BTreeNode *btn;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-2-3.dat");
    chidb_Btree_open(fname, db, &db->bt);
    chidb_Btree_getNodeByPage(db->bt, 2, &btn);

    btn_sanity_check(db->bt, btn, false);
    ck_assert(btn->page->npage == 2);
    ck_assert(btn->type == PGTYPE_TABLE_LEAF);
    ck_assert(btn->n_cells == 4);
    ck_assert(btn->free_offset == 16);
    ck_assert(btn->cells_offset == 480);
    ck_assert(get2byte(&btn->celloffset_array[0]) == 888);
    ck_assert(get2byte(&btn->celloffset_array[2]) == 752);
    ck_assert(get2byte(&btn->celloffset_array[4]) == 616);
    ck_assert(get2byte(&btn->celloffset_array[6]) == 480);

    chidb_Btree_freeMemNode(db->bt, btn);
    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_2_4)
{
    int rc;
    chidb *db;
    BTreeNode *btn;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-2-4.dat");
    chidb_Btree_open(fname, db, &db->bt);
    rc = chidb_Btree_getNodeByPage(db->bt, 6, &btn);
    ck_assert(rc == CHIDB_EPAGENO);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_2_5)
{
    chidb *db;
    BTreeNode *btn;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-2-5.dat");
    chidb_Btree_open(fname, db, &db->bt);

    for(int i=1; i<=5; i++)
    {
        chidb_Btree_getNodeByPage(db->bt, 2, &btn);
        btn_sanity_check(db->bt, btn, false);
        chidb_Btree_freeMemNode(db->bt, btn);
    }

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_2_6)
{
    chidb *db;
    BTreeNode *btn;

    db = malloc(sizeof(chidb));
    char *fname = create_copy(TESTFILE_STRINGS2, "btree-test-2-6.dat");
    chidb_Btree_open(fname, db, &db->bt);

    for(int i=1; i<=7; i++)
    {
        chidb_Btree_getNodeByPage(db->bt, 2, &btn);
        btn_sanity_check(db->bt, btn, false);
        chidb_Btree_freeMemNode(db->bt, btn);
    }

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


TCase* make_btree_2_tc(void)
{
    TCase *tc = tcase_create ("Step 2: Loading a B-Tree node from the file");
    tcase_add_test (tc, test_2_1);
    tcase_add_test (tc, test_2_2);
    tcase_add_test (tc, test_2_3);
    tcase_add_test (tc, test_2_4);
    tcase_add_test (tc, test_2_5);
    tcase_add_test (tc, test_2_6);

    return tc;
}
