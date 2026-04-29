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
