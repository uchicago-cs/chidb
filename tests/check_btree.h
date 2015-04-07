#include <stdlib.h>
#include <check.h>
#include "check_common.h"
#include "libchidb/btree.h"
#include "libchidb/util.h"

#define TESTFILE_STRINGS1 ("strings-1btree.sdb") // String database w/ five pages, single B-Tree
#define TESTFILE_STRINGS2 ("strings-2btree.sdb") // String database w/ seven pages, two B-Trees in 1, 5
#define TESTFILE_CORRUPT1 ("corruptheader-1.cdb") // Corrupt header
#define TESTFILE_CORRUPT2 ("corruptheader-2.cdb") // Corrupt header, in devious ways
#define TESTFILE_CORRUPT3 ("corruptheader-3.cdb") // Corrupt header, in even more devious ways

extern chidb_key_t file1_keys[];
extern char *file1_values[];
extern chidb_key_t file1_nvalues;

extern chidb_key_t bigfile_pkeys[];
extern chidb_key_t bigfile_ikeys[];
extern chidb_key_t bigfile_nvalues;

TCase* make_btree_1a_tc(void);
TCase* make_btree_1b_tc(void);
TCase* make_btree_2_tc(void);
TCase* make_btree_3_tc(void);
TCase* make_btree_4_tc(void);
TCase* make_btree_5_tc(void);
TCase* make_btree_6_tc(void);
TCase* make_btree_7_tc(void);
TCase* make_btree_8_tc(void);



FILE *copy(const char *from, const char *to);

void create_temp_file(const char *from);

void btn_sanity_check(BTree *bt, BTreeNode *btn, bool empty);

void btnNew_sanity_check(BTree *bt, BTreeNode *btn, uint8_t type);

void bt_sanity_check(BTree *bt, npage_t nroot);

void test_init_empty(BTree *bt, uint8_t type);

void test_new_node(BTree *bt, uint8_t type);

void test_values(BTree *bt, chidb_key_t *keys, char **values, chidb_key_t nkeys);

void insert_bigfile(chidb *db, int i);

void test_bigfile(chidb *db);

int chidb_Btree_findInIndex(BTree *bt, npage_t nroot, chidb_key_t ikey, chidb_key_t *pkey);

void test_index_bigfile(chidb *db, npage_t index_nroot);
