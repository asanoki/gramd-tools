dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(gramdtools, 1.0, [http://www.jovislab.com/gramdtools])

AC_CANONICAL_SYSTEM
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_INIT_AUTOMAKE()

AC_ARG_WITH([tcmalloc],
  [AS_HELP_STRING([--with-tcmalloc],
    [compile with tcmalloc])],
  [TCMALLOC_LDFLAGS="-ltcmalloc_minimal"],
  [TCMALLOC_LDFLAGS=''])
AC_SUBST([TCMALLOC_LDFLAGS])

AC_LANG_PUSH(C++)
AC_CHECK_HEADER(google/sparse_hash_map,[AC_DEFINE([HAVE_GOOGLE_SPARSE_HASH_MAP],[],["Have google::sparse_hash_map"])],[])
AC_LANG_POP

BOOST_REQUIRE([1.34])
BOOST_THREADS([])
BOOST_REGEX([mt])
BOOST_SYSTEM([mt])
BOOST_SERIALIZATION([mt])
BOOST_WSERIALIZATION([mt])
BOOST_PROGRAM_OPTIONS([mt])

LDFLAGS="$LDFLAGS $TCMALLOC_LDFLAGS"

AC_CONFIG_FILES(Makefile gramdtoolsmake/Makefile)
AC_OUTPUT

