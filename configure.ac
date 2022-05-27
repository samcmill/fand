dnl Package version
m4_define([fand_version_major], [0])
m4_define([fand_version_minor], [1])
m4_define([fand_version_micro], [0])

m4_define([fand_version],
          [fand_version_major.fand_version_minor.fand_version_micro])

AC_INIT([fand], [fand_version], [scott.andrew.mcmillan@gmail.com], [],
        [https://github.com/samcmill/fand])

AC_DEFINE_UNQUOTED([VERSION_MAJOR], [fand_version_major],
                   [Major version number])
AC_DEFINE_UNQUOTED([VERSION_MINOR], [fand_version_minor],
                   [Minor version number])
AC_DEFINE_UNQUOTED([VERSION_MICRO], [fand_version_micro],
                   [Micro version number])
AC_SUBST([VERSION_MAJOR], [fand_version_major])
AC_SUBST([VERSION_MINOR], [fand_version_minor])
AC_SUBST([VERSION_MICRO], [fand_version_micro])

dnl Internal logger
AC_DEFINE(LOGGER, "fand", [Logger name])

AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign])

AC_CONFIG_MACRO_DIRS([m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_code_coverage.m4])
m4_include([m4/ax_cxx_check_lib.m4])
m4_include([m4/ax_cxx_compile_stdcxx.m4])
m4_include([m4/ax_cxx_compile_stdcxx_14.m4])
m4_include([m4/ax_cxx_compile_stdcxx_17.m4])

AM_SILENT_RULES([yes])

AX_CODE_COVERAGE

AC_C_INLINE
AC_PROG_CXX
AM_PROG_AR

dnl Libtool
LT_INIT
#AC_REQUIRE_AUX_FILE([catch-driver])

dnl clang-format
AC_ARG_WITH([clang-format],
    [AS_HELP_STRING([--with-clang-format@<:@=ARG@:>@],
        [Enable re-formatting source code with clang-format. @<:@ARG=clang-format@:>@ ])],
        [case "${withval}" in
          yes ) prog_clang_format="clang-format" ;;
          no )  prog_clang_format="no" ;;
          * )   prog_clang_format="${withval}" ;;
          esac],
        [prog_clang_format="clang-format"])
AS_IF([test "x$prog_clang_format" != "xno"],
    [AC_PATH_PROG([CLANG_FORMAT], [$prog_clang_format], [])])
AM_CONDITIONAL([HAVE_CLANG_FORMAT], [test "x$CLANG_FORMAT" != "x"])

AC_LANG([C++])

dnl Check necessary types
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

dnl Prefer C++ 2017, but fallback to C++ 2014
AX_CXX_COMPILE_STDCXX([17], [noext], [optional])
if test "$HAVE_CXX17" -ne 1; then
  AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])
fi

AX_CXXFLAGS_WARN_ALL

dnl pthreads
AX_PTHREAD([LIBS="$PTHREAD_LIBS $LIBS"
            CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
            CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"])

dnl Check required features
AC_HEADER_STDC
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD

AC_CHECK_HEADERS([execution])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_FUNCS([ioctl])

dnl wassail
AC_ARG_ENABLE([static-wassail],
            [AS_HELP_STRING([--disable-static-wassail],
              [Disable static linking with the wassail library])])
AC_ARG_WITH([wassail],
            [AS_HELP_STRING([--with-wassail@<:@=ARG@:>@],
              [Specify the top-level path to the the wassail installation.])],
            [AC_SUBST(CPPFLAGS, "-I${withval}/include $CPPFLAGS")
             AS_IF([test "x$enable_static_wassail" != "xno"],
                   [LIBS="${withval}/lib/libwassail.a $LIBS"],
                   [LIBS="-L${withval}/lib -lwassail $LIBS"])],
            [])
AC_CHECK_HEADERS([wassail/wassail.hpp],,
                 AC_MSG_ERROR(Could not find wassail header))
AC_MSG_CHECKING(for libwassail)
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <wassail/wassail.hpp>],
                                [wassail::initialize()])],
               [AC_MSG_RESULT([yes])],
               [AC_MSG_ERROR(could not find libwassail)])
AC_SUBST(CPPFLAGS, "-DWASSAIL_LIBEXECDIR=\\\"${withval}/libexec/wassail\\\" $CPPFLAGS")

AC_ARG_VAR([SYSTEM], [Build for the targetted system only])
AS_IF([test "x$ac_cv_env_SYSTEM_set" = "xset"],
      [AC_DEFINE_UNQUOTED(FAND_SYSTEM, "$ac_cv_env_SYSTEM_value",
                          [Build for the targetted system only])])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
 Makefile
 configs/Makefile
 src/Makefile
 src/3rdparty/Makefile
])

AC_OUTPUT
