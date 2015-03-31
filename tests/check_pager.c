#include <stdlib.h>
#include <check.h>
#include "check_common.h"
#include "libchidb/pager.h"

#define NVALUES (256)
#define PAGE_SIZE (1024)
#define TESTFILESIZE (32768)
#define TESTFILE ("32k-of-zeroes.dat")
#define MAXPAGES (8)

#define NMULT (6)
uint8_t pagemult[] = {1,2,4,8,16,32};

uint16_t pagepos[] = {407, 685, 692, 847, 383, 813, 705, 113, 929, 576, 257, 602, 655, 17, 231, 787, 196,
                      765, 258, 123, 943, 481, 540, 117, 428, 580, 534, 541, 883, 255, 982, 488, 151, 766,
                      615, 380, 221, 367, 582, 820, 351, 67, 438, 0, 353, 922, 879, 560, 624, 658, 935, 601,
                      7, 868, 920, 613, 775, 473, 261, 607, 733, 290, 516, 210, 971, 309, 574, 317, 970,
                      188, 535, 462, 745, 671, 153, 708, 939, 69, 285, 127, 790, 938, 888, 603, 870, 405,
                      204, 191, 501, 672, 63, 1, 731, 1016, 695, 761, 393, 458, 312, 842, 976, 36, 721, 517,
                      817, 356, 612, 565, 120, 941, 163, 289, 840, 637, 953, 454, 1011, 1017, 1009, 588,
                      323, 512, 610, 915, 307, 513, 617, 991, 699, 852, 190, 952, 725, 138, 962, 509, 201,
                      228, 957, 238, 162, 183, 520, 321, 32, 328, 22, 339, 148, 39, 209, 583, 999, 755, 782,
                      399, 743, 94, 608, 760, 169, 333, 936, 52, 578, 96, 85, 638, 722, 609, 600, 546, 260,
                      537, 136, 58, 310, 890, 979, 84, 592, 147, 531, 251, 349, 661, 78, 240, 845, 394, 871,
                      51, 507, 805, 294, 694, 179, 304, 217, 283, 673, 83, 400, 998, 963, 135, 676, 646, 877,
                      472, 301, 129, 482, 259, 176, 980, 625, 56, 68, 523, 975, 751, 899, 563, 876, 944, 857,
                      895, 360, 114, 450, 442, 1005, 865, 904, 504, 814, 267, 165, 423, 1012, 954, 972, 344,
                      665, 780, 154, 550, 810, 395, 299, 80, 50, 698, 549, 216
                     };
uint8_t values[] = {237, 255, 57, 47, 157, 159, 21, 16, 168, 96, 0, 179, 225, 156, 142, 147, 31, 91, 32,
                    124, 188, 87, 76, 128, 248, 246, 206, 195, 72, 46, 187, 203, 83, 233, 106, 146, 120,
                    37, 132, 230, 229, 191, 221, 20, 220, 42, 64, 13, 250, 93, 117, 201, 210, 192, 138,
                    22, 68, 198, 219, 228, 245, 164, 204, 181, 205, 118, 53, 11, 88, 218, 149, 1, 209,
                    145, 197, 111, 242, 56, 99, 167, 170, 10, 73, 121, 148, 211, 14, 176, 139, 134, 82,
                    189, 19, 61, 194, 224, 80, 253, 70, 231, 109, 75, 24, 175, 130, 105, 86, 49, 251,
                    78, 123, 166, 173, 41, 2, 115, 97, 135, 232, 54, 58, 243, 66, 29, 252, 65, 34, 171,
                    12, 137, 217, 161, 196, 162, 69, 174, 33, 241, 227, 55, 6, 165, 48, 81, 254, 25,
                    15, 108, 18, 208, 207, 143, 213, 45, 95, 200, 4, 151, 101, 199, 140, 103, 186, 216,
                    172, 153, 40, 239, 63, 44, 133, 180, 27, 152, 102, 85, 116, 178, 182, 184, 107, 5,
                    114, 126, 77, 150, 98, 62, 223, 144, 212, 43, 155, 129, 92, 177, 226, 154, 94, 51,
                    79, 190, 59, 110, 234, 113, 7, 136, 235, 36, 8, 104, 89, 119, 100, 35, 52, 3, 249,
                    215, 236, 214, 169, 30, 84, 127, 122, 23, 26, 131, 240, 71, 17, 202, 28, 74, 160,
                    244, 183, 125, 38, 90, 158, 9, 222, 50, 163, 39, 193, 141, 238, 67, 247, 112, 60, 185
                   };

START_TEST (test_open)
{
    int rc;
    npage_t expected_numpages;
    Pager *pg;

    char *fname = create_copy(TESTFILE, "pager-test-open.dat");

    for(int i=0; i<NMULT; i++)
    {
        rc = chidb_Pager_open(&pg, fname);
        ck_assert(rc == CHIDB_OK);
        chidb_Pager_setPageSize(pg, PAGE_SIZE * pagemult[i]);
        expected_numpages = TESTFILESIZE / pg->page_size;
        ck_assert_int_eq(pg->n_pages, expected_numpages);
        chidb_Pager_close(pg);
    }

    delete_copy(fname);
}
END_TEST


START_TEST (test_read)
{
    int rc;
    Pager *pg;
    MemPage *page;

    char *fname = create_copy(TESTFILE, "pager-test-read.dat");

    for(int i=0; i<NMULT; i++)
    {
        rc = chidb_Pager_open(&pg, fname);
        ck_assert(rc == CHIDB_OK);
        chidb_Pager_setPageSize(pg, PAGE_SIZE * pagemult[i]);

        for(int j=1; j<=pg->n_pages; j++)
        {
            chidb_Pager_readPage(pg, j, &page);
            ck_assert(rc == CHIDB_OK);
            chidb_Pager_releaseMemPage(pg, page);
        }

        rc = chidb_Pager_readPage(pg, pg->n_pages + 1, &page);
        ck_assert(rc == CHIDB_EPAGENO);

        chidb_Pager_close(pg);
    }

    delete_copy(fname);
}
END_TEST


START_TEST (test_readwrite)
{
    int rc;
    npage_t npage;
    Pager *pg;
    MemPage *page;

    for(int i=0; i<NMULT; i++)
    {
        char *fname = create_tmp_file();

        rc = chidb_Pager_open(&pg, fname);
        ck_assert(rc == CHIDB_OK);

        chidb_Pager_setPageSize(pg, PAGE_SIZE * pagemult[i]);

        for(int j=1; j<=MAXPAGES; j++)
        {
            chidb_Pager_allocatePage(pg, &npage);
            ck_assert(npage == j);
        }

        for(int j=1; j<=MAXPAGES; j++)
        {
            chidb_Pager_readPage(pg, j, &page);
            for(int k=0; k<NVALUES; k++)
                page->data[pagepos[k]*(i+1)] = values[k];
            chidb_Pager_writePage(pg, page);
            chidb_Pager_releaseMemPage(pg, page);

            chidb_Pager_readPage(pg, j, &page);
            for(int k=0; k<NVALUES; k++)
                if(page->data[pagepos[k]*(i+1)] != values[k])
                {
                    ck_abort_msg("Incorrect value read from page");
                    break;
                }
            chidb_Pager_releaseMemPage(pg, page);
        }

        chidb_Pager_close(pg);
        delete_tmp_file(fname);
    }
}
END_TEST


Suite* make_pager_suite (void)
{
    Suite *s = suite_create ("Pager");

    TCase *tc_open = tcase_create ("Opening a file");
    tcase_add_test (tc_open, test_open);
    suite_add_tcase (s, tc_open);

    TCase *tc_read = tcase_create ("Reading from a file");
    tcase_add_test (tc_read, test_read);
    suite_add_tcase (s, tc_read);

    TCase *tc_readwrite = tcase_create ("Reading/writing a file");
    tcase_add_test (tc_readwrite, test_readwrite);
    suite_add_tcase (s, tc_readwrite);

    return s;
}

int main (void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create (make_pager_suite ());

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
