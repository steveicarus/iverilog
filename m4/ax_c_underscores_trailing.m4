AC_DEFUN([AX_C_UNDERSCORES_TRAILING],
[AC_CACHE_CHECK([for trailing underscores], ax_cv_c_underscores_trailing,
[_AX_C_UNDERSCORES_MATCH_IF([underscore_],
[AS_VAR_SET(ax_cv_c_underscores_trailing, yes)],
[AS_VAR_SET(ax_cv_c_underscores_trailing, no)])])
if test $ax_cv_c_underscores_trailing = yes; then
 AC_DEFINE([NEED_TU], [1], [Symbol names in object files produced by C compiler have trailing underscores.])
fi
])# AX_C_UNDERSCORES_TRAILING
