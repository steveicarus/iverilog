AC_DEFUN([AX_CONFIGURE_ARGS_NO_PREFIX],[
  ax_configure_args_no_prefix=
  ax_skip_next=no

  for ac_arg in $(echo "$ac_configure_args" | sed "s,',,g"); do
    if test "x$ax_skip_next" = "xyes"; then
      ax_skip_next=no
      continue
    fi

    case $ac_arg in
      --prefix)
        ax_skip_next=yes
        ;;
      --prefix=*)
        ;;
      *)
        ax_configure_args_no_prefix="$ax_configure_args_no_prefix '$ac_arg'"
        ;;
    esac
  done

  AC_SUBST([AX_CONFIGURE_ARGS_NO_PREFIX], [$ax_configure_args_no_prefix])
])
