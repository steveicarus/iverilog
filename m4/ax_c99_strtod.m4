AC_DEFUN([AX_C99_STRTOD],
[# On MinGW we need to jump through hoops to get a C99 compliant strtod().
 # mingw-w64 doesn't need this, and the 64-bit version doesn't support it.
case "${host}" in
    x86_64-w64-mingw32)
        ;;
    aarch64-w64-mingw32)
        ;;
    *-*-mingw32)
        LDFLAGS+=" -Wl,--undefined=___strtod,--wrap,strtod,--defsym,___wrap_strtod=___strtod"
        ;;
esac
])# AX_C99_STRTOD
