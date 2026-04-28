AC_DEFUN([AX_LD_RDYNAMIC],
[AC_MSG_CHECKING([for -rdynamic compiler flag])
rdynamic=-rdynamic
case "${host}" in

    *-*-netbsd*)
        rdynamic="-Wl,--export-dynamic"
        ;;

    *-*-openbsd*)
        rdynamic="-Wl,--export-dynamic"
        ;;

    *-*-solaris*)
        rdynamic=""
        ;;

    *-*-cygwin*)
        rdynamic=""
        ;;

    *-*-mingw*)
        rdynamic=""
        ;;

    *-*-hpux*)
        rdynamic="-E"
        ;;

    *-*-darwin*)
        rdynamic="-Wl,-all_load"
        strip_dynamic="-SX"
        ;;

esac
AC_SUBST(rdynamic)
AC_MSG_RESULT($rdynamic)
AC_SUBST(strip_dynamic)
# since we didn't tell them we're "checking", no good place to tell the answer
# AC_MSG_RESULT($strip_dynamic)
])# AX_LD_RDYNAMIC
