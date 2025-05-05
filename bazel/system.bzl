load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

SYSTEM_HEADERS_BASE = "_system"

def cc_system_headers(name, hdrs, strip_include_prefix='', **kwargs):
    """Copy headers in a folder and create a cc_library with system includes.

    This rule relies on strip_include_prefix behavior that adds a folder with virtual_includes
    and includes both iquote and isystem compiler options.

    The SYSTEM_HEADERS_BASE is just used to hide in the gendir the files.
    """
    outs = [
        '{}/{}'.format(SYSTEM_HEADERS_BASE, hdr)
        for hdr in hdrs
    ]
    [copy_file(
        name = "{}_{}".format(SYSTEM_HEADERS_BASE, hdr).replace('/', '_').replace('.', '_').replace('-', '_'),
        src = hdr,
        out = out,
    ) for hdr, out in zip(hdrs, outs)]
    includes = kwargs.pop('includes', default=[])
    includes = [
        '{}/{}'.format(SYSTEM_HEADERS_BASE, k)
        for k in includes
    ]
    native.cc_library(
        name = name,
        hdrs = outs,
        includes = includes,
        strip_include_prefix = '{}/{}'.format(SYSTEM_HEADERS_BASE, strip_include_prefix),
        **kwargs
    )
