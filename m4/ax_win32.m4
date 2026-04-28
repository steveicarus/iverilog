AC_DEFUN([AX_WIN32],
[AC_MSG_CHECKING([for Microsoft Windows])
AC_REQUIRE([AC_CANONICAL_HOST]) []dnl
case $host_os in
     *cygwin*) MINGW32=no;  WIN32=yes;;

      *mingw*) MINGW32=yes; WIN32=yes;;

            *) MINGW32=no;  WIN32=no;;
esac
AC_SUBST(MINGW32)
AC_SUBST(WIN32)
AC_MSG_RESULT($WIN32)
if test $WIN32 = yes; then
AC_MSG_CHECKING([for MinGW])
AC_MSG_RESULT($MINGW32)
fi
])# AX_WIN32
