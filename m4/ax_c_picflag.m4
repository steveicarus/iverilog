AC_DEFUN([AX_C_PICFLAG],
[AC_MSG_CHECKING([for flag to make position independent code])
PICFLAG=-fPIC
case "${host}" in

     *-*-cygwin*)
        PICFLAG=
        ;;

     *-*-mingw*)
        PICFLAG=
        ;;

     *-*-hpux*)
        PICFLAG=+z
        ;;

     *-*-solaris*)
        if test ${using_sunpro_c} = 1
        then
           PICFLAG=-G
        fi
        ;;
esac
AC_SUBST(PICFLAG)
AC_MSG_RESULT($PICFLAG)
])# AX_C_PICFLAG
