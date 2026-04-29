AC_DEFUN([AX_ENABLE_SUFFIX],
[AC_ARG_ENABLE([suffix],[AS_HELP_STRING([--enable-suffix],
                                        [Use/set the installation command suffix])],
               [true],[enable_suffix=no])
if test X$enable_suffix = Xyes; then
  install_suffix='-dev'
elif test X$enable_suffix = Xno; then
  install_suffix=''
else
  install_suffix="$enable_suffix"
fi
AC_SUBST(install_suffix)
])# AX_ENABLE_SUFFIX
