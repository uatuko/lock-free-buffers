# configure.ac
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([lock-free-buffers], [0.0.1-snapshot], [http://bugs.geniusse.com/])
AC_CONFIG_AUX_DIR([aux-build])
AC_CONFIG_MACRO_DIR([aux-build/m4])
AC_CONFIG_HEADERS([include/configs.h])

# init
AM_INIT_AUTOMAKE([foreign])
LT_INIT()

# check for programs
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_MKDIR_P
AC_PROG_LIBTOOL

# language
AC_LANG(C++)
AX_CXX_COMPILE_STDCXX_11

# doxygen
AC_CHECK_PROGS([DOXYGEN], [doxygen], [false])
AM_CONDITIONAL([HAVE_DOXYGEN], [test "x$DOXYGEN" != xfalse])
AM_COND_IF([HAVE_DOXYGEN],,
	AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
)

# cppunit
AM_PATH_CPPUNIT(1.12.1,
	[cppunit=true],
	[
		cppunit=false
		AC_MSG_WARN([Cppunit not found - continuing without unit test support])
	]
)
AM_CONDITIONAL([HAVE_CPPUNIT], [test x$cppunit = xtrue])

# TAP driver
AC_REQUIRE_AUX_FILE([tap-driver.sh])


AC_CONFIG_FILES([ \
	Makefile \
	doc/Makefile \
	doc/doxygen.cfg \
	include/Makefile \
	lib/Makefile \
	src/Makefile \
	src/examples/Makefile \
	test/Makefile
])

AC_OUTPUT

