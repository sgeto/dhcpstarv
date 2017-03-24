#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([dhcpstarv],[0.2.3],[https://github.com/sgeto/dhcpstarv/issues],[dhcpstarv],[https://github.com/sgeto/dhcpstarv])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.14 foreign subdir-objects no-dependencies nostdinc tar-ustar])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([src/config.h])
: ${CFLAGS="-Wall -Wextra -Wno-sign-compare -pedantic -O2"}

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h sys/ioctl.h sys/socket.h stddef.h net/ethernet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([inet_ntoa memset select socket strerror])

dnl Check for debug
dnl FIXME make it do any actual debuging
AC_MSG_CHECKING([whether to compile in debug mode])
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],[compile with extra debugging output @<:@default=no@:>@])],
    [use_debug=$enableval],
    [use_debug=no])
AC_MSG_RESULT([$use_debug])

if test "x$use_debug" = "xyes"; then
AC_DEFINE(VERBOSE_DHCP_DEBUG_OUTPUT, 1,
    [Define to print debug messages (e.g. DHCP packet field values).])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_RESULT([
$PACKAGE_STRING Configuration Complete
======================================

Configuration Summary
---------------------
    Prefix: ...................... ${prefix}
    CC: .......................... ${CC}
    CFLAGS:....................... ${CFLAGS}
    LD: .......................... ${LD}
    LDFLAGS: ..................... ${LDFLAGS}
    Compile in debug mode: ....... ${use_debug}

To override options, see ./configure --help

To disable silent building and print the full command line of every stage, run "make V=1"

Report bugs to https://github.com/sgeto/dhcpstarv/issues
])
