# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Description:
#   Icarus Verilog is a Verilog simulation and synthesis tool.
#   Use :iverilog and :vvp targets in your genrules.
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

load("//bazel:yacclex_helpers.bzl", "genyacc", "genlex")
load("//bazel:build_plugins.bzl", "vpi_binary")
load("//bazel:pseudo_configure.bzl", "pseudo_configure")
load("//bazel:system.bzl", "cc_system_headers")

# The only two exported labels are iverilog and vvp. They are enough
# to run simple simulations.
package(
    default_visibility = ["//visibility:private"],
    features = [
        "-layering_check",  # vpi/fastlz.c including itself
        "-parse_headers",  # missing: `size_t` definition in stab.h, `using string` in parse_api.h
    ],
)

licenses(["restricted"])  # GPLv2

exports_files([
    "LICENSE",
    "build-plugins",
])

filegroup(
    name = "system_vpis",
    srcs = [
        ":system_vpi",
        ":v2005_math_vpi",
        ":v2009_vpi",
        ":va_math_vpi",
        ":vhdl_sys_vpi",
        ":vhdl_textio_vpi",
        ":vpi_debug_vpi",
    ],
)

# This wrapper around iverilog compiler is to be used by
# simulations. A typical genrule will look similar to gen_hello.vvp
# below.
sh_binary(
    name = "iverilog",
    srcs = ["iverilog.sh"],
    data = [
        "vvp.conf",
        ":iverilog-bin",
        ":ivl",
        ":ivlpp",
        ":vvp_tgt",
    ],
    deps = ["@bazel_tools//tools/bash/runfiles"],
    output_licenses = ["unencumbered"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "vvp_conf",
    srcs = ["tgt-vvp/vvp.conf.in"],
    outs = ["vvp.conf"],
    cmd = "echo 'flag:VVP_EXECUTABLE=/unused' | cat $(location :tgt-vvp/vvp.conf.in) - > $@",
)

# This wrapper around vvp simulator is to be used by simulations. A
# typical genrule will look similar to run_hello below.
sh_binary(
    name = "vvp",
    srcs = ["vvp.sh"],
    data = [
        ":vvp-bin",
        ":system_vpis",
    ],
    output_licenses = ["unencumbered"],
    deps = ["@bazel_tools//tools/bash/runfiles"],
    visibility = ["//visibility:public"],
)

cc_system_headers(
    name = "tgt_vvp_config_header",
    hdrs = [
        "tgt-vvp/vvp_config.h",
    ],
)

cc_system_headers(
    name = "vpi_user_header",
    hdrs = [
        "vpi_user.h",
        "_pli_types.h",
    ],
)

cc_system_headers(
    name = "config_header",
    hdrs = [
        "config.h",
    ],
)

cc_library(
    name = "ivl-misc",
    srcs = [
        "libmisc/LineInfo.cc",
        "libmisc/StringHeap.cc",
    ],
    hdrs = [
        "libmisc/LineInfo.h",
        "libmisc/StringHeap.h",
    ],
    copts = ["-Wno-unused-variable"],
)

cc_binary(
    name = "ivl",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = ["elab_anet.cc"],
    ) + [
        ":config_h",
        "vvp/ivl_dlfcn.h",
        "_pli_types.h",
        ":lexor",
        ":lexor_keyword_cc",
        ":parse_y",
        ":syn-rules_y",
        ":version_tag_h",
    ],
    # Really poor code hygiene. Produces mounds of warnings.
    copts = ["-w"],
    includes = [
        "libmisc",
    ],
    linkopts = [
        "-ldl",
        "-Wl,--export-dynamic",
        "-Wl,-no-pie",
    ],
    deps = [
        ":ivl-misc",
    ],
)

genlex(
    name = "lexor",
    src = "lexor.lex",
    out = "lexor.cc",
)

genyacc(
    name = "parse_y",
    src = "parse.y",
    header_out = "parse.h",
    prefix = "VL",
    source_out = "parse.cc",
)

genyacc(
    name = "syn-rules_y",
    src = "syn-rules.y",
    header_out = "syn-rules.h",
    prefix = "syn_",
    source_out = "syn-rules.cc",
)

cc_binary(
    name = "iverilog-bin",
    srcs = glob([
        "driver/*.c",
        "driver/*.h",
    ]) + [
        ":config_h",
        "ivl_alloc.h",
        "version_base.h",
        ":version_tag_h",
        ":cflexor",
        ":cfparse_y",
    ],
    copts = [
        "-D_GNU_SOURCE",
        "-std=c11",
        "-fcommon",
        "-DIVL_LIB='\"\"'",
        "-DIVL_SUFFIX='\"\"'",
        "-DIVL_INCLUDE_INSTALL_DIR='\"\"'",
    ],
    includes = [
        "driver",
        "libmisc",
    ],
)

genlex(
    name = "cflexor",
    src = "driver/cflexor.lex",
    out = "driver/cflexor.c",
)

genyacc(
    name = "cfparse_y",
    src = "driver/cfparse.y",
    header_out = "driver/cfparse.h",
    prefix = "cf",
    source_out = "driver/cfparse.c",
)

cc_binary(
    name = "vvp-bin",
    srcs = glob([
        "vvp/*.cc",
        "vvp/*.h",
    ]) + [
        "ivl_alloc.h",
        "sv_vpi_user.h",
        "version_base.h",
        ":version_tag_h",
        ":gen_tables",
        ":vvp_flexor",
        ":vvp_parse_y",
        ":vvp_gen__vvp_config_h",
    ],
    copts = [
        "-O2",  # Optimized binary regardless of configuration.
        "-Wno-implicit-fallthrough",
    ],
    # Do not sort: dot last.
    includes = [
        "vvp_gen",
        "vvp",
    ],
    linkopts = [
        "-ldl",
        "-Wl,--export-dynamic",
    ],
    deps = [
        ":vpi_user_header",
        "@readline//:readline",
    ],
)

genyacc(
    name = "vvp_parse_y",
    src = "vvp/parse.y",
    header_out = "vvp_gen/parse.h",
    source_out = "vvp_gen/parse.cc",
)

genlex(
    name = "vvp_flexor",
    src = "vvp/lexor.lex",
    out = "vvp_gen/lexor.cc",
)

cc_binary(
    name = "draw_tt",
    srcs = ["vvp/draw_tt.c"],
)

genrule(
    name = "gen_tables",
    outs = ["vvp_gen/tables.cc"],
    cmd = "$(location :draw_tt) > $@",
    tools = [":draw_tt"],
)

cc_binary(
    name = "ivlpp",
    srcs = glob([
        "ivlpp/*.c",
        "ivlpp/*.h",
    ]) + [
        ":config_h",
        "ivl_alloc.h",
        "version_base.h",
        ":version_tag_h",
        ":ivlpp_lexor",
    ],
    copts = ["-Wno-unused-variable"],
    # Do not sort: dot last.
    includes = [
        "ivlpp",
    ],
)

genlex(
    name = "ivlpp_lexor",
    src = "ivlpp/lexor.lex",
    out = "ivlpp_gen/lexor.c",
)

vpi_binary(
    name = "system_vpi",
    srcs = [
        ":config_h",
        "ivl_alloc.h",
        "sv_vpi_user.h",
        "version_base.h",
        "vpi/fastlz.c",
        "vpi/fastlz.h",
        "vpi/fstapi.c",
        "vpi/fstapi.h",
        "vpi/lxt_write.c",
        "vpi/lxt_write.h",
        "vpi/lxt2_write.c",
        "vpi/lxt2_write.h",
        "vpi/lz4.c",
        "vpi/lz4.h",
        "vpi/mt19937int.c",
        "vpi/sdf_parse_priv.h",
        "vpi/sdf_priv.h",
        "vpi/stringheap.c",
        "vpi/stringheap.h",
        "vpi/sys_convert.c",
        "vpi/sys_countdrivers.c",
        "vpi/sys_darray.c",
        "vpi/sys_deposit.c",
        "vpi/sys_display.c",
        "vpi/sys_fileio.c",
        "vpi/sys_finish.c",
        "vpi/sys_fst.c",
        "vpi/sys_icarus.c",
        "vpi/sys_lxt.c",
        "vpi/sys_lxt2.c",
        "vpi/sys_plusargs.c",
        "vpi/sys_priv.c",
        "vpi/sys_priv.h",
        "vpi/sys_queue.c",
        "vpi/sys_random.c",
        "vpi/sys_random.h",
        "vpi/sys_random_mti.c",
        "vpi/sys_readmem.c",
        "vpi/sys_readmem_lex.h",
        "vpi/sys_scanf.c",
        "vpi/sys_sdf.c",
        "vpi/sys_table.c",
        "vpi/sys_time.c",
        "vpi/sys_vcd.c",
        "vpi/sys_vcdoff.c",
        "vpi/table_mod.c",
        "vpi/table_mod.h",
        "vpi/vams_simparam.c",
        "vpi/vcd_priv.c",
        "vpi/vcd_priv.h",
        "vpi/vcd_priv2.cc",
        "vpi/vpi_config.h",
        "vpi/wavealloca.h",
        ":table_mod_lexor_lex",
        ":table_mod_parse_y",
        ":vpi_sdf_lexor",
        ":vpi_sdfparse_y",
        ":vpi_sys_readmem_lex",
    ],
    out = "system.vpi",
    # Optimized binary regardless of configuration.
    copts = [
        "$(STACK_FRAME_UNLIMITED)",
        "-O2",
        # "-std=c11",
        # was for for-loop-indices, removed to allow vcd_priv2.cc to compile
    ],
    includes = [
        "vpi",
    ],
    # The code has atrocious const hygiene. Produces mounds of warnings.
    deps = [
        ":vpi_user_header",
        ":config_header",
        "@bzip2//:bz2",
        "@zlib//:zlib",
    ],
)

genyacc(
    name = "table_mod_parse_y",
    src = "vpi/table_mod_parse.y",
    header_out = "vpi/table_mod_parse.h",
    prefix = "tblmod",
    source_out = "vpi/table_mod_parse.c",
)

genlex(
    name = "table_mod_lexor_lex",
    src = "vpi/table_mod_lexor.lex",
    out = "vpi/table_mod_lexor.c",
)

genyacc(
    name = "vpi_sdfparse_y",
    src = "vpi/sdf_parse.y",
    header_out = "vpi/sdf_parse.h",
    prefix = "sdf",
    source_out = "vpi/sdf_parse.c",
)

genlex(
    name = "vpi_sdf_lexor",
    src = "vpi/sdf_lexor.lex",
    out = "vpi/sdf_lexor.c",
)

genlex(
    name = "vpi_sys_readmem_lex",
    src = "vpi/sys_readmem_lex.lex",
    out = "vpi/sys_readmem_lex.c",
)

vpi_binary(
    name = "vhdl_sys_vpi",
    srcs = [
        "ivl_alloc.h",
        "sv_vpi_user.h",
        "vpi/sys_priv.c",
        "vpi/sys_priv.h",
        "vpi/vhdl_table.c",
        "vpi/vpi_config.h",
    ],
    out = "vhdl_sys.vpi",
    copts = ["-O2"],
    includes = [
        "vpi",
    ],
    deps = [
        ":vpi_user_header",
    ],
)

vpi_binary(
    name = "vhdl_textio_vpi",
    srcs = [
        "ivl_alloc.h",
        "sv_vpi_user.h",
        "vpi/sys_priv.c",
        "vpi/sys_priv.h",
        "vpi/vhdl_textio.c",
        "vpi/vpi_config.h",
    ],
    out = "vhdl_textio.vpi",
    copts = ["-O2"],
    includes = [
        "vpi",
    ],
    deps = [
        ":vpi_user_header",
    ],
)

vpi_binary(
    name = "va_math_vpi",
    srcs = [
        "ivl_alloc.h",
        "vpi/va_math.c",
        "vpi/vpi_config.h",
    ],
    out = "va_math.vpi",
    copts = ["-O2"],  # Optimized binary regardless of configuration.
    includes = [
        "vpi",
    ],
    # The code has atrocious const hygiene. Produces mounds of warnings.
    deps = [
        ":vpi_user_header",
    ],
)

vpi_binary(
    name = "v2005_math_vpi",
    srcs = [
        "ivl_alloc.h",
        "sv_vpi_user.h",
        "vpi/sys_clog2.c",
        "vpi/v2005_math.c",
        "vpi/vpi_config.h",
    ],
    out = "v2005_math.vpi",
    copts = ["-O2"],  # Optimized binary regardless of configuration.
    includes = [
        "vpi",
    ],
    # The code has atrocious const hygiene. Produces mounds of warnings.
    deps = [
        ":vpi_user_header",
    ],
)

vpi_binary(
    name = "vpi_debug_vpi",
    srcs = ["vpi/vpi_debug.c"],
    out = "vpi_debug.vpi",
    copts = ["-O2"],
    includes = [
        "vpi",
    ],
    deps = [
        ":vpi_user_header",
    ],
)

vpi_binary(
    name = "v2009_vpi",
    srcs = [
        "ivl_alloc.h",
        "sv_vpi_user.h",
        "vpi/sys_priv.c",
        "vpi/sys_priv.h",
        "vpi/v2009_array.c",
        "vpi/v2009_bitvec.c",
        "vpi/v2009_enum.c",
        "vpi/v2009_string.c",
        "vpi/v2009_table.c",
        "vpi/vpi_config.h",
    ],
    out = "v2009.vpi",
    copts = [
        "-O2",  # Optimized binary regardless of configuration.
    ],
    includes = [
        "vpi",
    ],
    deps = [
        ":vpi_user_header",
    ],
)

vpi_binary(
    name = "vvp_tgt",
    srcs = glob([
        "tgt-vvp/*.c",
        "tgt-vvp/*.h",
    ]) + [
        "ivl_alloc.h",
        "ivl_target.h",
        "version_base.h",
        ":version_tag_h",
        ":tgt_vvp__vvp_config_h",
    ],
    out = "vvp.tgt",
    copts = [
        "-std=c11",
        "-Wno-implicit-function-declaration",
        "-Wno-int-conversion",
    ],
    includes = [
        "tgt-vvp",
    ],
    # The code has atrocious const hygiene. Produces mounds of warnings.
)

genrule(
    name = "_pli_types_h",
    srcs = ["_pli_types.h.in"],
    outs = ["_pli_types.h"],
    cmd = "cat $(location :_pli_types.h.in) | sed 's/# undef HAVE_INTTYPES_H/# define HAVE_INTTYPES_H 1/' > $@",
)

genrule(
    name = "lexor_keyword_cc",
    srcs = ["lexor_keyword.gperf"],
    tools = ["@gperf//:gperf"],
    outs = ["lexor_keyword.cc"],
    cmd = "$(location @gperf//:gperf) -o -i 7 -C -k 1-4,6,9,$$ -H keyword_hash -N check_identifier -t $(location :lexor_keyword.gperf) > $@",
    message = "Generating perfect hash function from $(SRCS)",
)

# In the following genrules we do an extremely crude approximation of a
# configuration step -- workable now given the limited set of
# platforms/environments we intend to target.

HAVE_CONFIG_SUFFIXES = "TIMES|IOSFWD|GETOPT_H|INTTYPES_H|DLFCN_H|LIBREADLINE|LIBZ|LIBBZ2|LROUND|SYS_WAIT_H|ALLOCA_H|FSEEKO|LIBPTHREAD|REALPATH"
HAVE_CONFIG_RE = "HAVE_(%s)" % HAVE_CONFIG_SUFFIXES

DEFS = [
    "HAVE_IOSFWD",
    "HAVE_DLFCN_H",
    "HAVE_GETOPT_H",
    "HAVE_LIBREADLINE",
    "HAVE_READLINE_READLINE_H",
    "HAVE_LIBHISTORY",
    "HAVE_READLINE_HISTORY_H",
    "HAVE_INTTYPES_H",
    "HAVE_LROUND",
    "HAVE_LLROUND",
    "HAVE_NAN",
    "UINT64_T_AND_ULONG_SAME",
    "HAVE_SYS_RESOURCE_H",
    "LINUX"
]

pseudo_configure(
    name = "tgt_vvp__vvp_config_h",
    src = "tgt-vvp/vvp_config.h.in",
    out = "tgt-vvp/vvp_config.h",
    defs = [
        "HAVE_STDINT_H",
        "HAVE_INTTYPES_H",
        "_LARGEFILE_SOURCE"
    ],
    mappings = {},
)

pseudo_configure(
    name = "config_h",
    src = "config.h.in",
    out = "config.h",
    defs = [
        "HAVE_TIMES",
        "HAVE_IOSFWD",
        "HAVE_GETOPT_H",
        "HAVE_INTTYPES_H",
        "HAVE_DLFCN_H",
        "HAVE_LIBREADLINE",
        "HAVE_LIBZ",
        "HAVE_LIBBZ2",
        "HAVE_LROUND",
        "HAVE_SYS_WAIT_H",
        "HAVE_ALLOCA_H",
        "HAVE_FSEEKO",
        "HAVE_LIBPTHREAD",
        "HAVE_REALPATH"
    ],
    mappings = {},
)

genrule(
    name = "vpi__vpi_config_h",
    srcs = ["vpi/vpi_config.h.in"],
    outs = ["vpi/vpi_config.h"],
    cmd = "perl -p -e 's/# undef (\\w+)/#define $$1 1/' $< > $@",
    message = "Configuring vpi/vpi_config.h.in",
)

pseudo_configure(
    name = "vvp_gen__vvp_config_h",
    src = "vvp/config.h.in",
    out = "vvp_gen/config.h",
    defs = DEFS,
    mappings = {
        "SIZEOF_UNSIGNED_LONG_LONG": "8",
        "SIZEOF_UNSIGNED_LONG": "8",
        "SIZEOF_UNSIGNED": "4",
        "SIZEOF_VOID_P": "8",
        "USE_READLINE": "",
        "USE_HISTORY": "",
        "MODULE_DIR": '"."',
        "__STDC_FORMAT_MACROS": "",
        "TIME_FMT_O": '"lo"',
        "TIME_FMT_U": '"lu"',
        "TIME_FMT_X": '"lx"',
        "UL_AND_TIME64_SAME": "",
        "i64round": "lround",
        "nan(x)": "(NAN)",
        "INFINITY": "HUGE_VAL",
        "LU": '""',
        "TU": '""'
    },
)

genrule(
    name = "version_tag_h",
    outs = ["version_tag.h"],
    cmd = "\n".join([
        "cat <<'EOF' >$@",
        '#ifndef __VERSION_TAG_H_',
        '#define __VERSION_TAG_H_',
        '#define VERSION_TAG "v12_0"',
        '#endif  // __VERSION_TAG_H_',
        "EOF",
    ]),
)

# Trivial integration tests to confirm iverilog is minimally functional.

genrule(
    name = "hello_vvp",
    srcs = ["hello.v"],
    outs = ["hello.vvp"],
    cmd = (
        "$(location :iverilog) " +
        "-o $@ " +
        "$<"
    ),
    tools = [
        ":iverilog",
        ":vvp",  # to resolve module include
    ],
)

vpi_binary(
    name = "hello_vpi",
    srcs = ["hello_vpi.c"],
    out = "hello.vpi",
    deps = [
        ":vpi_user_header",
    ],
)

genrule(
    name = "run_hello",
    srcs = ["hello.vvp"],
    outs = ["hello.out"],
    cmd = (
        "$(location :vvp) " +
        "-M$$(dirname $(location :hello_vpi)) " +
        "-mhello $< > $@ "
    ),
    tools = [
        ":hello_vpi",
        ":vvp",
    ],
)

sh_test(
    name = "hello_verilog_test",
    srcs = [":hello_verilog_test.sh"],
    args = ["$(location :run_hello)"],
    data = [":run_hello"],
)
