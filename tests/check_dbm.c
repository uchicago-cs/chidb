#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <dirent.h>
#include <chidb/chidb.h>
#include "libchidb/dbm.h"
#include "libchidb/dbm-file.h"
#include "libchidb/dbm-types.h"
#include "check_common.h"

// Make this array bigger if we ever have more than 1024 DBM tests
char *dbm_tests[1024];

START_TEST (test_dbm)
{
    int rc;
    chidb_dbm_file_t *dbmf;

    rc = chidb_dbm_file_load2(dbm_tests[_i], &dbmf, DATABASES_DIR, GENERATED_DIR, true);
    ck_assert_msg(rc == CHIDB_OK, "Could not load DBM file %s\n", dbm_tests[_i]);

    list_iterator_start(&dbmf->queryResults);
    do
    {
        rc = chidb_dbm_file_run(dbmf);

        ck_assert_msg(rc == CHIDB_ROW || rc == CHIDB_DONE, "Error while running DBM file %s\n", dbm_tests[_i]);

        if(rc == CHIDB_ROW)
        {
            char *actualRR = chidb_stmt_rr_str(&dbmf->stmt, ' ');
            if(!list_iterator_hasnext(&dbmf->queryResults))
                ck_abort_msg("DBM program produced a result row [%s] but none was expected", actualRR);

            char *expectedRR = (char*) list_iterator_next(&dbmf->queryResults);

            ck_assert_msg(strcmp(actualRR, expectedRR) == 0,
                    "DBM program produced result row [%s] but expected [%s]", actualRR, expectedRR);

            free(actualRR);
        }
    } while (rc != CHIDB_DONE);
    if(list_iterator_hasnext(&dbmf->queryResults))
        ck_abort_msg("DBM finished, but more results rows were expected");

    list_iterator_stop(&dbmf->queryResults);


    list_iterator_start(&dbmf->registers);
    while (list_iterator_hasnext(&dbmf->registers))
    {
        chidb_dbm_file_register_t *freg = (chidb_dbm_file_register_t*) list_iterator_next(&dbmf->registers);
        uint32_t nReg = freg->nReg;
        chidb_dbm_register_t *expected = &freg->reg, *actual;

        ck_assert_msg(nReg < dbmf->stmt.nReg, "Cannot check value of R_%i. DBM only has %i registers.", nReg, dbmf->stmt.nReg);

        actual = &dbmf->stmt.reg[nReg];

        ck_assert_msg(expected->type == actual->type, "Expected R_%i to be of type %s, but it is of type %s", nReg,
                regtype_to_str(expected->type), regtype_to_str(actual->type));
        if(freg->has_value)
        {
            switch(expected->type)
            {
            case REG_INT32:
                ck_assert_msg(expected->value.i == actual->value.i,
                        "Expected register %i to have value %i but it has value %i", nReg, expected->value.i, actual->value.i);
                break;
            case REG_STRING:
                ck_assert_msg(strcmp(expected->value.s, actual->value.s) == 0,
                        "Expected register %i to have value '%s' but it has value '%s'", nReg, expected->value.s, actual->value.s);
                break;
            case REG_BINARY:
                /* TODO: Check value. Currently not supported by DMB file format. */
                break;
            case REG_UNSPECIFIED:
            case REG_NULL:
                /* Nothing to do, since we've already checked that the types of the registers match */
                break;
            }
        }

    }
    list_iterator_stop(&dbmf->registers);

    rc = chidb_dbm_file_close(dbmf);
}
END_TEST



int main (void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create (NULL);

    int i = 0;
    DIR *dir1 = opendir (DBM_PROGRAMS_DIR);
    Suite *s;
    if (dir1)
    {
        struct dirent *ent1;
        while ((ent1 = readdir (dir1)) != NULL)
        {
            if (ent1->d_type == DT_DIR && strcmp(ent1->d_name, ".") && strcmp(ent1->d_name, ".."))
            {
                char *dirname2 = malloc(strlen(DBM_PROGRAMS_DIR) + strlen(ent1->d_name) + 2);
                sprintf(dirname2, "%s%s/", DBM_PROGRAMS_DIR, ent1->d_name);

                DIR *dir2 = opendir (dirname2);
                if(dir2)
                {
                    int snamelen = strlen("dbm-") + strlen(ent1->d_name) + 1;
                    char *sname = malloc(snamelen);
                    snprintf(sname, snamelen, "dbm-%s", ent1->d_name);
                    s = suite_create (sname);

                    struct dirent *ent2;
                    while ((ent2 = readdir (dir2)) != NULL)
                    {
                        if (ent2->d_type == DT_REG)
                        {
                            dbm_tests[i] = malloc(strlen(dirname2) + strlen(ent2->d_name) + 1);
                            sprintf(dbm_tests[i], "%s%s", dirname2, ent2->d_name);

                            TCase *tc = tcase_create (strdup(ent2->d_name));
                            tcase_add_loop_test(tc, test_dbm, i, i+1);
                            suite_add_tcase (s, tc);

                            i++;
                        }
                    }
                }
                else
                {
                    printf("Could not open DBM programs directory: %s\n", dirname2);
                    exit(1);
                }

                srunner_add_suite(sr, s);
                closedir(dir2);
            }
        }

        closedir(dir1);
    } else
    {
        printf("Could not open DBM programs directory: " DBM_PROGRAMS_DIR "\n");
        exit(1);
    }

    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
