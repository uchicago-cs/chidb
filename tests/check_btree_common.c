#include <stdlib.h>
#include <check.h>
#include "check_btree.h"


void btn_sanity_check(BTree *bt, BTreeNode *btn, bool empty)
{
    ck_assert(btn->page->npage >= 1);
    int header_offset = btn->page->npage==1? 100:0;

    ck_assert(btn->type == PGTYPE_TABLE_INTERNAL || btn->type == PGTYPE_TABLE_LEAF  || btn->type == PGTYPE_INDEX_INTERNAL  || btn->type == PGTYPE_INDEX_LEAF);
    ck_assert(btn->n_cells >= 0);

    switch(btn->type)
    {
    case PGTYPE_TABLE_INTERNAL:
    case PGTYPE_INDEX_INTERNAL:
        ck_assert(btn->free_offset == header_offset + INTPG_CELLSOFFSET_OFFSET + (btn->n_cells * 2));
        ck_assert(btn->celloffset_array == btn->page->data + header_offset + INTPG_CELLSOFFSET_OFFSET);
        break;
    case PGTYPE_TABLE_LEAF:
    case PGTYPE_INDEX_LEAF:
        ck_assert(btn->free_offset == header_offset + LEAFPG_CELLSOFFSET_OFFSET + (btn->n_cells * 2));
        ck_assert(btn->celloffset_array == btn->page->data + header_offset + LEAFPG_CELLSOFFSET_OFFSET);
        break;
    }

    ck_assert(btn->cells_offset >= btn->free_offset);
    ck_assert(btn->cells_offset <= bt->pager->page_size);

    for(int i=0; i<btn->n_cells; i++)
    {
        uint16_t cell_offset = get2byte(&btn->celloffset_array[i*2]);
        ck_assert(cell_offset >= btn->cells_offset);
        ck_assert(cell_offset <= bt->pager->page_size);
    }

    if (!empty && (btn->type == PGTYPE_TABLE_INTERNAL || btn->type == PGTYPE_INDEX_INTERNAL))
    {
        ck_assert(btn->right_page > 1);
        ck_assert(btn->right_page <= bt->pager->n_pages);
    }
}

void btnNew_sanity_check(BTree *bt, BTreeNode *btn, uint8_t type)
{
    bool leaf = (type == PGTYPE_TABLE_LEAF || type == PGTYPE_INDEX_LEAF);

    btn_sanity_check(bt, btn, true);
    ck_assert(btn->type == type);
    ck_assert(btn->n_cells == 0);
    ck_assert(btn->free_offset == (leaf? LEAFPG_CELLSOFFSET_OFFSET : INTPG_CELLSOFFSET_OFFSET));
    ck_assert(btn->cells_offset == bt->pager->page_size);
    ck_assert(btn->celloffset_array == (uint8_t*) (btn->page->data + (leaf? LEAFPG_CELLSOFFSET_OFFSET : INTPG_CELLSOFFSET_OFFSET)));
}


void bt_sanity_check(BTree *bt, npage_t nroot)
{
    return;
}

void test_init_empty(BTree *bt, uint8_t type)
{
    BTreeNode *btn;
    npage_t npage;

    chidb_Pager_allocatePage(bt->pager, &npage);
    chidb_Btree_initEmptyNode(bt, npage, type);

    chidb_Btree_getNodeByPage(bt, npage, &btn);
    btnNew_sanity_check(bt, btn, type);
    chidb_Btree_freeMemNode(bt, btn);
}

void test_new_node(BTree *bt, uint8_t type)
{
    BTreeNode *btn;
    npage_t npage;

    chidb_Btree_newNode(bt, &npage, type);
    chidb_Btree_getNodeByPage(bt, npage, &btn);
    btnNew_sanity_check(bt, btn, type);
    chidb_Btree_freeMemNode(bt, btn);
}

void test_values(BTree *bt, chidb_key_t *keys, char **values, chidb_key_t nkeys)
{
    uint16_t size;
    uint8_t *data;
    int rc;

    for(int i = 0; i<nkeys; i++)
    {
        rc = chidb_Btree_find(bt, 1, keys[i], &data, &size);
        ck_assert(rc == CHIDB_OK);
        ck_assert(size == 128);
        ck_assert(!strcmp((char *) data, values[i]));
    }
}


void insert_bigfile(chidb *db, int i)
{
    int rc;
    int datalen = ((bigfile_pkeys[i] % 3) + 1) * 64;
    uint8_t buf[192];

    for(int j=0; j<48; j++)
        put4byte(buf + (4*j), bigfile_ikeys[i]);

    rc = chidb_Btree_insertInTable(db->bt, 1, bigfile_pkeys[i], buf, datalen);
    ck_assert(rc == CHIDB_OK);

}

void test_bigfile(chidb *db)
{
    int rc;

    for(int i=0; i<bigfile_nvalues; i++)
    {
        uint8_t* buf;
        uint16_t size;
        uint8_t data[192];
        int datalen = ((bigfile_pkeys[i] % 3) + 1) * 64;

        for(int j=0; j<48; j++)
            put4byte(data + (4*j), bigfile_ikeys[i]);

        rc = chidb_Btree_find(db->bt, 1, bigfile_pkeys[i], &buf, &size);
        ck_assert(rc == CHIDB_OK);
        ck_assert(size == datalen);
        ck_assert(!memcmp(buf, data, datalen));
    }
}

int chidb_Btree_findInIndex(BTree *bt, npage_t nroot, chidb_key_t ikey, chidb_key_t *pkey)
{
    BTreeNode *btn;

    chidb_Btree_getNodeByPage(bt, nroot, &btn);

    if (btn->type == PGTYPE_INDEX_LEAF)
    {
        for(int i = 0; i<btn->n_cells; i++)
        {
            BTreeCell btc;

            chidb_Btree_getCell(btn, i, &btc);
            if(btc.key == ikey)
            {
                *pkey = btc.fields.indexLeaf.keyPk;
                return CHIDB_OK;
            }
        }
        return CHIDB_ENOTFOUND;
    }
    else if (btn->type == PGTYPE_INDEX_INTERNAL)
    {
        for(int i = 0; i<btn->n_cells; i++)
        {
            BTreeCell btc;

            chidb_Btree_getCell(btn, i, &btc);

            if(btc.key == ikey)
            {
                *pkey = btc.fields.indexInternal.keyPk;
                return CHIDB_OK;
            }

            if(ikey <= btc.key)
                return chidb_Btree_findInIndex(bt, btc.fields.indexInternal.child_page, ikey, pkey);
        }
        return chidb_Btree_findInIndex(bt, btn->right_page, ikey, pkey);
    }

    chidb_Btree_freeMemNode(bt, btn);

    return CHIDB_OK;
}

void test_index_bigfile(chidb *db, npage_t index_nroot)
{
    int rc;

    for(int i=0; i<bigfile_nvalues; i++)
    {
        uint8_t* buf;
        uint16_t size;
        uint8_t data[192];
        chidb_key_t pkey;

        rc = chidb_Btree_findInIndex(db->bt, index_nroot, bigfile_ikeys[i], &pkey);
        ck_assert(rc == CHIDB_OK);

        int datalen = ((pkey % 3) + 1) * 64;

        for(int j=0; j<48; j++)
            put4byte(data + (4*j), bigfile_ikeys[i]);

        rc = chidb_Btree_find(db->bt, 1, pkey, &buf, &size);
        ck_assert(rc == CHIDB_OK);
        ck_assert(size == datalen);
        ck_assert(!memcmp(buf, data, datalen));
    }
}

