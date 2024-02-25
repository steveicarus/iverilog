
Files With Version Information
==============================

These are the only files that have version information in them:

* version_base.h    -- This should be the 1 source for version info.
* version_tag.h     -- Generated automatically with git tag information.
* verilog.spec      -- Used to stamp RPM packages

When versions are changed, the above files need to be edited to account for
the new version information. The following used to have verion information in
them, but now their version information is generated:

The version_tag.h file is generated from git tag information using
the "make version" target, or automatically if the version_tag.h
file doesn't exist at all. This implies that a "make version" is
something worth doing when you do a "git pull" or create commits.

The files below are now edited by the makefile and the version.exe program:

* iverilog-vpi.man    -- The .TH tag has a version string
* driver/iverilog.man -- The .TH tag has a version string
* driver-vpi/res.rc   -- Used to build Windows version stamp
* vvp/vvp.man         -- The .TH tag has a version string

This now includes version_base.h to get the version:

* vpi/vams_simparam.c -- Hard coded result to simulatorVersion query

This is actually a test file list that is specific to a major version.
The regression test scripts query the version of the compiler to infer
that it must include this list of tests. For example, for version 12.x
of the compiler, the needs to be an ivltest/regress-v12.list file that
lists the tests that are specific to that version.

* ivltests/regress-XXX.list -- Version specific regression tests
