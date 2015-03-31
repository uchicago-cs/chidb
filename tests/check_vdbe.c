#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <dirent.h>
#include "check_common.h"
#include "vdbe_tester.h"

// Make this array bigger if we ever have more than 1024 VDBE tests
char *vdbe_tests[1024];

START_TEST (test_vdbe)
{
    printf("%s\n", vdbe_tests[_i]);
    run_vdbe_file(vdbe_tests[_i]);
}
END_TEST

Suite* make_vdbe_suite (void)
{
    Suite *s = suite_create ("VDBE");

    int start, i = 0;
    DIR *dir1 = opendir (VDBE_PROGRAMS_DIR);
    if (dir1) {
        struct dirent *ent1;
        while ((ent1 = readdir (dir1)) != NULL)
        {
            if (ent1->d_type == DT_DIR)
            {
                TCase *tc = tcase_create (ent1->d_name);
                start = i;

                char *dirname2 = malloc(strlen(VDBE_PROGRAMS_DIR) + strlen(ent1->d_name) + 2);
                sprintf(dirname2, "%s%s/", VDBE_PROGRAMS_DIR, ent1->d_name);

                DIR *dir2 = opendir (dirname2);
                if(dir2)
                {
                    struct dirent *ent2;
                    while ((ent2 = readdir (dir2)) != NULL)
                    {
                        if (ent2->d_type == DT_REG)
                        {
                            vdbe_tests[i] = malloc(strlen(dirname2) + strlen(ent2->d_name) + 1);
                            sprintf(vdbe_tests[i], "%s%s", dirname2, ent2->d_name);
                            i++;
                        }
                    }
                }
                else
                {
                    printf("Could not open VDBE programs directory: %s\n", dirname2);
                    exit(1);
                }

                tcase_add_loop_test(tc, test_vdbe, start, i);

                suite_add_tcase (s, tc);
                closedir(dir2);
            }
        }
        closedir(dir1);

        for(int j=0; j < i; j++)
        {
            printf("%i: %s\n", j, vdbe_tests[j]);
        }
    } else
    {
        printf("Could not open VDBE programs directory: " VDBE_PROGRAMS_DIR "\n");
        exit(1);
    }

    return s;
}

int main (void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create (make_vdbe_suite ());

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
