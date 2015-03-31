#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([chidb], [1.0], [do-not-email@example.org])
AC_CONFIG_SRCDIR([src/libchidb/api.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])


# Checks for programs.
AC_PROG_CC

AC_PROG_LEX
AC_PATH_PROG([LEX_INST], $LEX)
if test -z "$LEX_INST"; then
   AC_MSG_ERROR([lex/flex not found])
fi

AC_PROG_YACC
AC_PATH_PROG([YACC_INST], $YACC)
if test -z "$YACC_INST"; then
   AC_MSG_ERROR([yacc/bison not found])
fi

AM_PROG_CC_C_O

LT_INIT


# Checks for libedit.
AC_CHECK_LIB([edit], [el_init], , AC_MSG_ERROR([libedit not found]))
AC_CHECK_HEADER([histedit.h], ,AC_MSG_ERROR([libedit header files not found]))

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h libintl.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h strings.h sys/time.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero gettimeofday memset strchr strdup])
PKG_CHECK_MODULES(CHECK, [check >= 0.9.14],,[AC_MSG_RESULT([no, testing is disabled])]) 

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
