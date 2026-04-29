AC_DEFUN([AX_LD_EXTRALIBS],
[AC_MSG_CHECKING([for extra libs needed])
EXTRALIBS=
case "${host}" in
     *-*-cygwin* )
        if test "$MINGW32" = "yes"; then
            EXTRALIBS="-liberty"
        fi
        ;;
esac
AC_SUBST(EXTRALIBS)
AC_MSG_RESULT($EXTRALIBS)
])# AX_LD_EXTRALIBS
