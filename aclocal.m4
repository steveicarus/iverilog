
# AX_ENABLE_SUFFIX
# ----------------
#  Create the configure option --enable-suffix[=suffix] to generate suffix
# strings for the installed commands. This allows for shared installs of
# different builds. Remember to change the default suffix string to some
# value appropriate for the current version.
AC_DEFUN([AX_ENABLE_SUFFIX],
[AC_ARG_ENABLE([suffix],[AC_HELP_STRING([--enable-suffix],
                                        [Use/set the installation command suffix])],
               [true],[enable_suffix=no])
if test X$enable_suffix = Xyes; then
  install_suffix='-0.9'
elif test X$enable_suffix = Xno; then
  install_suffix=''
else
  install_suffix="$enable_suffix"
fi
AC_SUBST(install_suffix)
])# AX_ENABLE_SUFFIX

# _AX_C_UNDERSCORES_MATCH_IFELSE(PATTERN, ACTION-IF-MATCH, ACTION-IF-NOMATCH)
# ------------------------------
# Sub-macro for AX_C_UNDERSCORES_LEADING and AX_C_UNDERSCORES_TRAILING.
# Unwarranted assumptions:
#   - the object file produced by AC_COMPILE_IFELSE is called "conftest.$ac_objext"
#   - the nm(1) utility is available, and its name is "nm".
AC_DEFUN([_AX_C_UNDERSCORES_MATCH_IF],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([void underscore(void){}])],
[AS_IF([nm conftest.$ac_objext|grep $1 >/dev/null 2>/dev/null],[$2],[$3])],
[AC_MSG_ERROR([underscore test crashed])]
)])


# AX_C_UNDERSCORES_LEADING
# ---------------------------------
# Check if symbol names in object files produced by C compiler have
# leading underscores. Define NEED_LU if so.
AC_DEFUN([AX_C_UNDERSCORES_LEADING],
[AC_CACHE_CHECK([for leading underscores], ax_cv_c_underscores_leading,
[_AX_C_UNDERSCORES_MATCH_IF([_underscore],
[AS_VAR_SET(ax_cv_c_underscores_leading, yes)],
[AS_VAR_SET(ax_cv_c_underscores_leading, no)])])
if test $ax_cv_c_underscores_leading = yes -a "$CYGWIN" != "yes" -a "$MINGW32" != "yes"; then
 AC_DEFINE([NEED_LU], [1], [Symbol names in object files produced by C compiler have leading underscores.])
fi
])# AX_C_UNDERSCORES_LEADING


# AX_C_UNDERSCORES_TRAILING
# ---------------------------------
# Check if symbol names in object files produced by C compiler have
# trailing underscores.  Define NEED_TU if so.
AC_DEFUN([AX_C_UNDERSCORES_TRAILING],
[AC_CACHE_CHECK([for trailing underscores], ax_cv_c_underscores_trailing,
[_AX_C_UNDERSCORES_MATCH_IF([underscore_],
[AS_VAR_SET(ax_cv_c_underscores_trailing, yes)],
[AS_VAR_SET(ax_cv_c_underscores_trailing, no)])])
if test $ax_cv_c_underscores_trailing = yes; then
 AC_DEFINE([NEED_TU], [1], [Symbol names in object files produced by C compiler have trailing underscores.])
fi
])# AX_C_UNDERSCORES_TRAILING

# AX_WIN32
# --------
# Combined check for several flavors of Microsoft Windows so
 # their "issues" can be dealt with
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

# AX_LD_EXTRALIBS
# ---------------
# mingw needs to link with libiberty.a, but cygwin alone can't tolerate it
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

# AX_LD_SHAREDLIB_OPTS
# --------------------
# linker options when building a shared library
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
        shared="-bundle -undefined suppress"
        ;;

     *-*-darwin*)
        shared="-bundle -undefined suppress -flat_namespace"
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

# AX_C_PICFLAG
# ------------
# The -fPIC flag is used to tell the compiler to make position
# independent code. It is needed when making shared objects.
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

# AX_LD_RDYNAMIC
# --------------
# The -rdynamic flag is used by iverilog when compiling the target,
# to know how to export symbols of the main program to loadable modules
# that are brought in by -ldl
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

# AX_CPP_PRECOMP
# --------------
AC_DEFUN([AX_CPP_PRECOMP],
[# Darwin requires -no-cpp-precomp
case "${host}" in
    *-*-darwin*)
        CPPFLAGS="-no-cpp-precomp $CPPFLAGS"
        CFLAGS="-no-cpp-precomp $CFLAGS"
        ;;
esac
])# AX_CPP_PRECOMP

# AX_C99_STRTOD
# -------------
AC_DEFUN([AX_C99_STRTOD],
[# On MinGW we need to jump through hoops to get a C99 compliant strtod().
case "${host}" in
    *-*-mingw*)
        LDFLAGS+=" -Wl,--undefined=___strtod,--wrap,strtod,--defsym,___wrap_strtod=___strtod"
        ;;
esac
])# AX_C99_STRTOD

# When config.status generates a header, we must update the stamp-h file.
# This file resides in the same directory as the config header
# that is generated.  The stamp file name are based on the header name.

# Autoconf calls _AC_AM_CONFIG_HEADER_HOOK (when defined) in the
# loop where config.status creates the headers, so we can generate
# our stamp files there.
AC_DEFUN([_AC_AM_CONFIG_HEADER_HOOK],
[
_config_header=$1
_stamp_name=stamp-`expr //$_config_header : '.*/\([[^./]]*\)\.[[^./]]*$'`-h
echo "timestamp for $_config_header" > `AS_DIRNAME(["$_config_header"])`/[]$_stamp_name
]) #_AC_AM_CONFIG_HEADER_HOOK
