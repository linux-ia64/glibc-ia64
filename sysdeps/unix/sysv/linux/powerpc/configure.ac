sinclude(./aclocal.m4)dnl Autoconf lossage
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/powerpc/.

AC_CACHE_CHECK(whether $CC $CFLAGS -mlong-double-128 uses IBM extended format,
	       libc_cv_mlong_double_128ibm, [dnl
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -mlong-double-128"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <float.h>]], [[
#if LDBL_MANT_DIG != 106
# error "compiler doesn't implement IBM extended format of long double"
#endif
long double foobar (long double x) { return x; }]])],
	       libc_cv_mlong_double_128ibm=yes,
	       libc_cv_mlong_double_128ibm=no)
CFLAGS="$save_CFLAGS"])

if test "$libc_cv_mlong_double_128ibm" = no; then
  CFLAGS="$CFLAGS -mabi=ibmlongdouble"
fi

LIBC_LINKER_FEATURE([--no-tls-get-addr-optimize], [-Wl,--no-tls-get-addr-optimize],
                    [libc_cv_tls_get_addr_optimize=yes], [libc_cv_tls_get_addr_optimize=no])
LIBC_CONFIG_VAR([have-tls-get-addr-optimize], [$libc_cv_tls_get_addr_optimize])

ldd_rewrite_script=sysdeps/unix/sysv/linux/powerpc/ldd-rewrite.sed
