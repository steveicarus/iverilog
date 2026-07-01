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
