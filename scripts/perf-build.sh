#!/bin/sh
# Build the performance-tuned iverilog/vvp installation: clang +
# ThinLTO + PGO, trained on the benchmark suite. The default flags favor
# maximum runtime performance; set PERF_BASEFLAGS to override them (for
# example, use -fno-omit-frame-pointer when collecting perf call graphs).
#
# Usage: scripts/perf-build.sh BUILDDIR [JOBS]
#
# Requirements: clang/clang++ 15+ with matching llvm-ar, llvm-ranlib
# and llvm-profdata (override with CLANG=, CLANGXX=, LLVM_AR=,
# LLVM_RANLIB=, LLVM_PROFDATA=, PERF_BASEFLAGS=). If the host's plain
# clang++ is shadowed by another toolchain (e.g. Xilinx Vitis), pass
# absolute paths. Extra include/library dirs for bison/readline/bzip2
# etc. can be passed with PERF_CPPFLAGS= and PERF_LDFLAGS=.
#
# The result installs into BUILDDIR/final/install. The user-facing
# iverilog-vpi keeps LTO flags out of user VPI module builds (see
# driver-vpi/Makefile.in).

set -eu

BUILDDIR=${1:?usage: perf-build.sh BUILDDIR [JOBS]}
JOBS=${2:-$(nproc)}
SRCDIR=$(cd "$(dirname "$0")/.." && pwd)

CLANG=${CLANG:-clang}
CLANGXX=${CLANGXX:-clang++}
LLVM_AR=${LLVM_AR:-llvm-ar}
LLVM_RANLIB=${LLVM_RANLIB:-llvm-ranlib}
LLVM_PROFDATA=${LLVM_PROFDATA:-llvm-profdata}
PERF_CPPFLAGS=${PERF_CPPFLAGS:-}
PERF_LDFLAGS=${PERF_LDFLAGS:-}

BASEFLAGS=${PERF_BASEFLAGS:--O3 -g -fomit-frame-pointer -flto=thin}
PROFDIR="$BUILDDIR/profiles"
PROFDATA="$BUILDDIR/pgo.profdata"

mkdir -p "$BUILDDIR"
BUILDDIR=$(cd "$BUILDDIR" && pwd)

[ -f "$SRCDIR/configure" ] || (cd "$SRCDIR" && sh autoconf.sh)

build_tree () { # dir extra-cflags
    mkdir -p "$1/install"
    cd "$1"
    "$SRCDIR/configure" --prefix="$1/install" \
        CC="$CLANG" CXX="$CLANGXX" AR="$LLVM_AR" RANLIB="$LLVM_RANLIB" \
        CPPFLAGS="$PERF_CPPFLAGS" \
        LDFLAGS="$PERF_LDFLAGS $BASEFLAGS $2" \
        CFLAGS="$BASEFLAGS $2" \
        CXXFLAGS="$BASEFLAGS $2"
    make -j"$JOBS" BUILDCC="$CLANG"
    make install BUILDCC="$CLANG"
    cd "$SRCDIR"
}

echo "== Stage 1: instrumented build"
build_tree "$BUILDDIR/instrumented" "-fprofile-generate=$PROFDIR"

echo "== Stage 2: train on the benchmark suite"
python3 "$SRCDIR/benchmarks/run.py" \
    --iverilog "$BUILDDIR/instrumented/install/bin/iverilog" \
    --runs 1 --warmups 0 --label pgo-train \
    --output "$BUILDDIR/pgo-train.json"

"$LLVM_PROFDATA" merge -output="$PROFDATA" "$PROFDIR"/*.profraw

echo "== Stage 3: final PGO build"
build_tree "$BUILDDIR/final" \
    "-fprofile-use=$PROFDATA -Wno-profile-instr-unprofiled -Wno-profile-instr-out-of-date"

echo "== Done: $BUILDDIR/final/install"
