#!/usr/bin/env bash

wrap_task() {
  echo "::group::$@"
  "$@"
  echo '::endgroup::'
}

wrap_task ./autoconf.sh
wrap_task ./configure
wrap_task make install

which iverilog

wrap_task make check
