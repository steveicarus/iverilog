
# AX_ENABLE_SUFFIX
# ----------------
#  Create the configure option --enable-suffix[=suffix] to generate suffix
# strings for the installed commands. This allows for shared installs of
# different builds. Remember to change the default suffix string to some
# value appropriate for the current version.
AC_DEFUN([AX_ENABLE_SUFFIX],
[AC_ARG_ENABLE([suffix],[AS_HELP_STRING([--enable-suffix],
                                        [Use/set the installation command suffix])],
               [true],[enable_suffix=no])
if test X$enable_suffix = Xyes; then
  install_suffix='-0.10'
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
#   - the object file produced by AC_COMPILE_IFELSE is called
#     "conftest.$ac_objext"
#   - the nm(1) utility or an equivalent is available, and its name
#     is defined by the $NM variable.
AC_DEFUN([_AX_C_UNDERSCORES_MATCH_IF],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([void underscore(void){}])],
[AS_IF([$NM conftest.$ac_objext|grep $1 >/dev/null 2>/dev/null],[$2],[$3])],
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


# AX_C99_STRTOD
# -------------
AC_DEFUN([AX_C99_STRTOD],
[# On MinGW we need to jump through hoops to get a C99 compliant strtod().
 # mingw-w64 doesn't need this, and the 64-bit version doesn't support it.
case "${host}" in
    x86_64-w64-mingw32)
        ;;
    *-*-mingw32)
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

# ===========================================================================
#   https://www.gnu.org/software/autoconf-archive/ax_prog_cc_for_build.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PROG_CC_FOR_BUILD
#
# DESCRIPTION
#
#   This macro searches for a C compiler that generates native executables,
#   that is a C compiler that surely is not a cross-compiler. This can be
#   useful if you have to generate source code at compile-time like for
#   example GCC does.
#
#   The macro sets the CC_FOR_BUILD and CPP_FOR_BUILD macros to anything
#   needed to compile or link (CC_FOR_BUILD) and preprocess (CPP_FOR_BUILD).
#   The value of these variables can be overridden by the user by specifying
#   a compiler with an environment variable (like you do for standard CC).
#
#   It also sets BUILD_EXEEXT and BUILD_OBJEXT to the executable and object
#   file extensions for the build platform, and GCC_FOR_BUILD to `yes' if
#   the compiler we found is GCC. All these variables but GCC_FOR_BUILD are
#   substituted in the Makefile.
#
# LICENSE
#
#   Copyright (c) 2008 Paolo Bonzini <bonzini@gnu.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 18

AU_ALIAS([AC_PROG_CC_FOR_BUILD], [AX_PROG_CC_FOR_BUILD])
AC_DEFUN([AX_PROG_CC_FOR_BUILD], [dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl

dnl Use the standard macros, but make them use other variable names
dnl
pushdef([ac_cv_prog_CPP], ac_cv_build_prog_CPP)dnl
pushdef([ac_cv_prog_cc_c89], ac_cv_build_prog_cc_c89)dnl
pushdef([ac_cv_prog_gcc], ac_cv_build_prog_gcc)dnl
pushdef([ac_cv_prog_cc_works], ac_cv_build_prog_cc_works)dnl
pushdef([ac_cv_prog_cc_cross], ac_cv_build_prog_cc_cross)dnl
pushdef([ac_cv_prog_cc_g], ac_cv_build_prog_cc_g)dnl
pushdef([ac_cv_c_compiler_gnu], ac_cv_build_c_compiler_gnu)dnl
pushdef([ac_cv_exeext], ac_cv_build_exeext)dnl
pushdef([ac_cv_objext], ac_cv_build_objext)dnl
pushdef([ac_exeext], ac_build_exeext)dnl
pushdef([ac_objext], ac_build_objext)dnl
pushdef([CC], CC_FOR_BUILD)dnl
pushdef([CPP], CPP_FOR_BUILD)dnl
pushdef([GCC], GCC_FOR_BUILD)dnl
pushdef([CFLAGS], CFLAGS_FOR_BUILD)dnl
pushdef([CPPFLAGS], CPPFLAGS_FOR_BUILD)dnl
pushdef([EXEEXT], BUILD_EXEEXT)dnl
pushdef([LDFLAGS], LDFLAGS_FOR_BUILD)dnl
pushdef([OBJEXT], BUILD_OBJEXT)dnl
pushdef([host], build)dnl
pushdef([host_alias], build_alias)dnl
pushdef([host_cpu], build_cpu)dnl
pushdef([host_vendor], build_vendor)dnl
pushdef([host_os], build_os)dnl
pushdef([ac_cv_host], ac_cv_build)dnl
pushdef([ac_cv_host_alias], ac_cv_build_alias)dnl
pushdef([ac_cv_host_cpu], ac_cv_build_cpu)dnl
pushdef([ac_cv_host_vendor], ac_cv_build_vendor)dnl
pushdef([ac_cv_host_os], ac_cv_build_os)dnl
pushdef([ac_tool_prefix], ac_build_tool_prefix)dnl
pushdef([am_cv_CC_dependencies_compiler_type], am_cv_build_CC_dependencies_compiler_type)dnl
pushdef([am_cv_prog_cc_c_o], am_cv_build_prog_cc_c_o)dnl
pushdef([cross_compiling], cross_compiling_build)dnl

cross_compiling_build=no

ac_build_tool_prefix=
AS_IF([test -n "$build"],      [ac_build_tool_prefix="$build-"],
      [test -n "$build_alias"],[ac_build_tool_prefix="$build_alias-"])

AC_LANG_PUSH([C])
AC_PROG_CC
_AC_COMPILER_EXEEXT
_AC_COMPILER_OBJEXT
AC_PROG_CPP

dnl Restore the old definitions
dnl
popdef([cross_compiling])dnl
popdef([am_cv_prog_cc_c_o])dnl
popdef([am_cv_CC_dependencies_compiler_type])dnl
popdef([ac_tool_prefix])dnl
popdef([ac_cv_host_os])dnl
popdef([ac_cv_host_vendor])dnl
popdef([ac_cv_host_cpu])dnl
popdef([ac_cv_host_alias])dnl
popdef([ac_cv_host])dnl
popdef([host_os])dnl
popdef([host_vendor])dnl
popdef([host_cpu])dnl
popdef([host_alias])dnl
popdef([host])dnl
popdef([OBJEXT])dnl
popdef([LDFLAGS])dnl
popdef([EXEEXT])dnl
popdef([CPPFLAGS])dnl
popdef([CFLAGS])dnl
popdef([GCC])dnl
popdef([CPP])dnl
popdef([CC])dnl
popdef([ac_objext])dnl
popdef([ac_exeext])dnl
popdef([ac_cv_objext])dnl
popdef([ac_cv_exeext])dnl
popdef([ac_cv_c_compiler_gnu])dnl
popdef([ac_cv_prog_cc_g])dnl
popdef([ac_cv_prog_cc_cross])dnl
popdef([ac_cv_prog_cc_works])dnl
popdef([ac_cv_prog_cc_c89])dnl
popdef([ac_cv_prog_gcc])dnl
popdef([ac_cv_prog_CPP])dnl

dnl restore global variables ac_ext, ac_cpp, ac_compile,
dnl ac_link, ac_compiler_gnu (dependant on the current
dnl language after popping):
AC_LANG_POP([C])

dnl Finally, set Makefile variables
dnl
AC_SUBST(BUILD_EXEEXT)dnl
AC_SUBST(BUILD_OBJEXT)dnl
AC_SUBST([CFLAGS_FOR_BUILD])dnl
AC_SUBST([CPPFLAGS_FOR_BUILD])dnl
AC_SUBST([LDFLAGS_FOR_BUILD])dnl
])
