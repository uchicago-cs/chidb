
#
# Common
#
ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I$(srcdir)/include -I$(srcdir)/src/simclist/ \
            -g3 -Wall -std=gnu99 -ggdb -D_GNU_SOURCE
AM_LDFLAGS = 
AM_YFLAGS = -d

noinst_LTLIBRARIES = libsimclist.la
lib_LTLIBRARIES = libchidb.la libchisql.la
BUILT_SOURCES =
MOSTLYCLEANFILES =

# Silence making even more by suppressing "Entering/Leaving" directory messages
# Use "make V=1" to reenable 
PRINT_DIRECTORY_FLAGS_1=
PRINT_DIRECTORY_FLAGS_0=--no-print-directory
PRINT_DIRECTORY_FLAGS_=$(PRINT_DIRECTORY_FLAGS_$(AM_DEFAULT_VERBOSITY))
AM_MAKEFLAGS = $(PRINT_DIRECTORY_FLAGS_$(V))


#
# We define our own rules for lexx and yacc, since automake's ylwrap
# is a pain to work with, and was meant for much older versions of lex/yacc
# (ylwrap is supposed to be optional in future versions of automake;
# when that happens, we should remove these rules)
#
AM_V_LEX = $(am__v_LEX_$(V))
am__v_LEX_ = $(am__v_LEX_$(AM_DEFAULT_VERBOSITY))
am__v_LEX_0 = @echo "  LEX     " $*.l;

%-lexer.c %-lexer.h: %.l
	$(AM_V_LEX)$(LEX) -o $*-lexer.c --header-file=$*-lexer.h $*.l

AM_V_YACC = $(am__v_YACC_$(V))
am__v_YACC_ = $(am__v_YACC_$(AM_DEFAULT_VERBOSITY))
am__v_YACC_0 = @echo "  YACC    " $*.y;

%-parser.c %-parser.h: %.y
	$(AM_V_YACC)$(YACC) -o $*-parser.c --defines=$*-parser.h $*.y

#    
# simclist
#
libsimclist_la_SOURCES = src/simclist/simclist.c
 

#
# libchisql
#

libchisql_la_SOURCES = \
                        src/libchisql/create.c \
                        src/libchisql/ra.c \
                        src/libchisql/literal.c \
                        src/libchisql/common.c \
                        src/libchisql/insert.c \
                        src/libchisql/condition.c \
                        src/libchisql/expression.c \
                        src/libchisql/column.c \
                        src/libchisql/delete.c \
                        src/libchisql/sra.c \
                        src/libchisql/sql-parser.c \
                        src/libchisql/sql-lexer.c
libchisql_la_CFLAGS = $(AM_CFLAGS)

BUILT_SOURCES += \
                 $(srcdir)/src/libchisql/sql-lexer.c \
                 $(srcdir)/src/libchisql/sql-parser.c

MOSTLYCLEANFILES += \
                    $(srcdir)/src/libchisql/sql-lexer.[ch] \
                    $(srcdir)/src/libchisql/sql-parser.[ch]




#
# libchidb
#

libchidb_la_SOURCES = \
                        src/libchidb/api.c \
                        src/libchidb/util.c \
                        src/libchidb/btree.c \
                        src/libchidb/pager.c \
                        src/libchidb/record.c \
                        src/libchidb/dbm.c \
                        src/libchidb/dbm-file.c \
                        src/libchidb/dbm-ops.c \
                        src/libchidb/dbm-cursor.c \
                        src/libchidb/codegen.c \
                        src/libchidb/optimizer.c \
                        src/libchidb/log.c 
libchidb_la_CFLAGS = $(AM_CFLAGS)
libchidb_la_LIBADD = libsimclist.la libchisql.la
libchidb_la_DEPENDENCIES = libsimclist.la libchisql.la


#    
# chidb shell
#
bin_PROGRAMS = chidb
chidb_SOURCES = \
                 src/shell/main.c \
                 src/shell/shell.c \
                 src/shell/commands.c 
chidb_LDADD = libsimclist.la libchidb.la libchisql.la
chidb_DEPENDENCIES = libsimclist.la libchidb.la libchisql.la


#
# tests
#
CHIDB_BUILT_TESTS = tests/check_btree tests/check_dbrecord tests/check_dbm \
                    tests/check_pager tests/check_utils
TESTS = $(CHIDB_BUILT_TESTS) 
check_PROGRAMS = $(CHIDB_BUILT_TESTS)

tests_check_btree_SOURCES = tests/check_btree.c \
                               tests/check_btree_common.c \
                               tests/check_btree_files.c \
                               tests/check_btree_1a.c \
                               tests/check_btree_1b.c \
                               tests/check_btree_2.c \
                               tests/check_btree_3.c \
                               tests/check_btree_4.c \
                               tests/check_btree_5.c \
                               tests/check_btree_6.c \
                               tests/check_btree_7.c \
                               tests/check_btree_8.c \
                               tests/check_common.c
tests_check_btree_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS) -I${srcdir}/src/ -DTEST_DIR="\"$(srcdir)/tests/\""
tests_check_btree_LDADD = libchidb.la $(CHECK_LIBS) 

tests_check_dbrecord_SOURCES = tests/check_dbrecord.c
tests_check_dbrecord_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS) -I${srcdir}/src/
tests_check_dbrecord_LDADD = libchidb.la $(CHECK_LIBS) 

tests_check_dbm_SOURCES = tests/check_dbm.c  \
                           tests/check_common.c
tests_check_dbm_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS) -I${srcdir}/src/ -DTEST_DIR="\"$(srcdir)/tests/\""
tests_check_dbm_LDADD = libchidb.la $(CHECK_LIBS) 

tests_check_pager_SOURCES = tests/check_pager.c \
                            tests/check_common.c
tests_check_pager_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS) -I${srcdir}/src/ -DTEST_DIR="\"$(srcdir)/tests/\""
tests_check_pager_LDADD = libchidb.la $(CHECK_LIBS) 

tests_check_utils_SOURCES = tests/check_utils.c
tests_check_utils_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS) -I${srcdir}/src/
tests_check_utils_LDADD = libchidb.la $(CHECK_LIBS) 

