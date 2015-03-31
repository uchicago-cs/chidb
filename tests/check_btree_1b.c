#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

START_TEST (test_1b_1)
{
    int rc;
    chidb *db;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);
    ck_assert(db->bt->pager->n_pages == 1);
    ck_assert(db->bt->pager->page_size == 1024);

    rc = chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


START_TEST (test_1b_2)
{
    int rc;
    chidb *db;
    MemPage *page;
    uint8_t *rawpage;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    rc = chidb_Pager_readPage(db->bt->pager, 1, &page);
    ck_assert(rc == CHIDB_OK);
    rawpage = page->data;

    if(strcmp((char *) rawpage, "SQLite format 3") || rawpage[18] != 1 || rawpage[19] != 1 ||
            rawpage[20] != 0 || rawpage[21] != 64 || rawpage[22] != 32 || rawpage[23] != 32 ||
            get4byte(&rawpage[32]) != 0 || get4byte(&rawpage[36]) != 0 || get4byte(&rawpage[44]) != 1 ||
            get4byte(&rawpage[52]) != 0 || get4byte(&rawpage[56]) != 1 || get4byte(&rawpage[64]) != 0 ||
            get4byte(&rawpage[48]) != 20000)
        ck_abort_msg("File header is not well-formed.");

    if(rawpage[100] != PGTYPE_TABLE_LEAF || get2byte(&rawpage[101]) != 108 || get2byte(&rawpage[103]) != 0 ||
            get2byte(&rawpage[105]) != 1024 || rawpage[107] != 0)
        ck_abort_msg("Page 1 header is not well-formed.");

    rc = chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


START_TEST (test_1b_3)
{
    int rc;
    chidb *db;

    char *fname = create_tmp_file();
    db = malloc(sizeof(chidb));
    rc = chidb_Btree_open(fname, db, &db->bt);
    ck_assert(rc == CHIDB_OK);

    rc = chidb_Btree_close(db->bt);
    delete_tmp_file(fname);
    free(db);
}
END_TEST


TCase* make_btree_1b_tc(void)
{
    TCase *tc = tcase_create ("Step 1b: Opening a new chidb file");
    tcase_add_test (tc, test_1b_1);
    tcase_add_test (tc, test_1b_2);
    tcase_add_test (tc, test_1b_3);

    return tc;
}

