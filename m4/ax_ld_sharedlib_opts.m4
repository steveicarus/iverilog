AC_DEFUN([AX_LD_SHAREDLIB_OPTS],
[AC_MSG_CHECKING([for shared library link flag])
shared=-shared
case "${host}" in
     *-*-cygwin*)
        shared="-shared -Wl,--enable-auto-image-base"
        ;;

     *-*-mingw*)
        shared="-shared -Wl,--enable-auto-image-base"
        ;;

     *-*-hpux*)
        shared="-b"
        ;;

     *-*-darwin1.[0123])
        shared="-bundle -undefined dynamic_lookup"
        ;;

     *-*-darwin*)
        shared="-bundle -undefined dynamic_lookup -flat_namespace"
        ;;

     *-*-solaris*)
        if test ${using_sunpro_c} = 1
        then
           shared="-G"
        fi
        ;;
esac
AC_SUBST(shared)
AC_MSG_RESULT($shared)
])# AX_LD_SHAREDLIB_OPTS
