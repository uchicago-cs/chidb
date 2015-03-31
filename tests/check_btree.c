#include <stdlib.h>
#include <check.h>
#include "check_btree.h"

Suite* make_btree_suite (void)
{
    Suite *s = suite_create ("B-Trees");

    suite_add_tcase (s, make_btree_1a_tc());
    suite_add_tcase (s, make_btree_2_tc());
    suite_add_tcase (s, make_btree_3_tc());
    suite_add_tcase (s, make_btree_1b_tc());
    suite_add_tcase (s, make_btree_4_tc());
    suite_add_tcase (s, make_btree_5_tc());
    suite_add_tcase (s, make_btree_6_tc());
    suite_add_tcase (s, make_btree_7_tc());
    suite_add_tcase (s, make_btree_8_tc());

    return s;
}

int main (void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create (make_btree_suite ());

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
