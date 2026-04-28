AC_DEFUN([AX_C_UNDERSCORES_LEADING],
[AC_CACHE_CHECK([for leading underscores], ax_cv_c_underscores_leading,
[_AX_C_UNDERSCORES_MATCH_IF([_underscore],
[AS_VAR_SET(ax_cv_c_underscores_leading, yes)],
[AS_VAR_SET(ax_cv_c_underscores_leading, no)])])
if test $ax_cv_c_underscores_leading = yes -a "$CYGWIN" != "yes" -a "$MINGW32" != "yes"; then
 AC_DEFINE([NEED_LU], [1], [Symbol names in object files produced by C compiler have leading underscores.])
fi
])# AX_C_UNDERSCORES_LEADING
