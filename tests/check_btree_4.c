#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

START_TEST (test_4_1)
{
    chidb *db;
    BTreeNode *btn;
    BTreeCell btc;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-4-1.dat");
    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    chidb_Btree_getNodeByPage(db->bt, 1, &btn);

    chidb_Btree_getCell(btn, 1, &btc);
    ck_assert(btc.type == PGTYPE_TABLE_INTERNAL);
    ck_assert(btc.key == 35);
    ck_assert(btc.fields.tableInternal.child_page == 3);

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_4_2)
{
    chidb *db;
    BTreeNode *btn;
    BTreeCell btc;

    char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-4-2.dat");
    db = malloc(sizeof(chidb));
    chidb_Btree_open(fname, db, &db->bt);
    chidb_Btree_getNodeByPage(db->bt, 5, &btn);

    chidb_Btree_getCell(btn, 2, &btc);
    ck_assert(btc.type == PGTYPE_TABLE_LEAF);
    ck_assert(btc.key == 127);
    ck_assert(btc.fields.tableLeaf.data_size == 128);
    ck_assert(!memcmp(btc.fields.tableLeaf.data, "foo127", 6));

    chidb_Btree_close(db->bt);
    delete_copy(fname);
    free(db);
}
END_TEST


START_TEST (test_4_3)
{
    chidb *db;
    BTreeNode *btn;
    BTreeCell btc;
    ncell_t old_ncell;
    uint16_t old_freeoffset;

    for(ncell_t cellpos = 0; cellpos < 4; cellpos++)
    {
        char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-4-3.dat");

        db = malloc(sizeof(chidb));
        chidb_Btree_open(fname, db, &db->bt);

        chidb_Btree_getNodeByPage(db->bt, 1, &btn);
        old_ncell = btn->n_cells;
        old_freeoffset = btn->free_offset;
        btc.key = 500;
        btc.type = PGTYPE_TABLE_INTERNAL;
        btc.fields.tableInternal.child_page = 6;
        chidb_Btree_insertCell(btn, cellpos, &btc);
        btn_sanity_check(db->bt, btn, false);
        ck_assert(btn->n_cells == old_ncell + 1);
        ck_assert(btn->free_offset == old_freeoffset + 2);

        chidb_Btree_writeNode(db->bt, btn);

        chidb_Btree_getNodeByPage(db->bt, 1, &btn);
        btn_sanity_check(db->bt, btn, false);
        chidb_Btree_getCell(btn, cellpos, &btc);
        ck_assert(btc.type == PGTYPE_TABLE_INTERNAL);
        ck_assert(btc.key == 500);
        ck_assert(btc.fields.tableInternal.child_page == 6);

        chidb_Btree_close(db->bt);
        delete_copy(fname);
        free(db);
    }
}
END_TEST


START_TEST (test_4_4)
{
    chidb *db;
    BTreeNode *btn;
    BTreeCell btc;
    ncell_t old_ncell;
    uint16_t old_freeoffset;
    char string[128];

    for(ncell_t cellpos = 0; cellpos < 4; cellpos++)
    {
        char *fname = create_copy(TESTFILE_STRINGS1, "btree-test-4-4.dat");

        db = malloc(sizeof(chidb));
        chidb_Btree_open(fname, db, &db->bt);

        chidb_Btree_getNodeByPage(db->bt, 3, &btn);
        old_ncell = btn->n_cells;
        old_freeoffset = btn->free_offset;
        btc.key = 123;
        btc.type = PGTYPE_TABLE_LEAF;
        btc.fields.tableLeaf.data_size = 128;
        bzero(string, 128);
        strcpy(string, "foobar");
        btc.fields.tableLeaf.data = (uint8_t*) string;
        chidb_Btree_insertCell(btn, cellpos, &btc);
        btn_sanity_check(db->bt, btn, false);
        ck_assert(btn->n_cells == old_ncell + 1);
        ck_assert(btn->free_offset == old_freeoffset + 2);

        chidb_Btree_writeNode(db->bt, btn);

        chidb_Btree_getNodeByPage(db->bt, 3, &btn);
        btn_sanity_check(db->bt, btn, false);
        chidb_Btree_getCell(btn, cellpos, &btc);
        ck_assert(btc.type == PGTYPE_TABLE_LEAF);
        ck_assert(btc.key == 123);
        ck_assert(btc.fields.tableLeaf.data_size == 128);
        ck_assert(!memcmp(btc.fields.tableLeaf.data, "foobar", 6));

        chidb_Btree_close(db->bt);
        delete_copy(fname);
        free(db);
    }
}
END_TEST


TCase* make_btree_4_tc(void)
{
    TCase *tc = tcase_create ("Step 4: Manipulating B-Tree cells");
    tcase_add_test (tc, test_4_1);
    tcase_add_test (tc, test_4_2);
    tcase_add_test (tc, test_4_3);
    tcase_add_test (tc, test_4_4);

    return tc;
}

