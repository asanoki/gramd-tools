dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(gramdtools, 1.0, [http://www.jovislab.com/gramdtools])

AC_CANONICAL_SYSTEM
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_INIT_AUTOMAKE()

BOOST_REQUIRE([1.34])
BOOST_THREADS([mt])
BOOST_REGEX([mt])
BOOST_SYSTEM([mt])
BOOST_SERIALIZATION([mt])
BOOST_WSERIALIZATION([mt])
BOOST_PROGRAM_OPTIONS([mt])

AC_CONFIG_FILES(Makefile gramdtoolsmake/Makefile)
AC_OUTPUT

