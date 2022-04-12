
Files With Version Information
==============================

These are the only files that have version information in them:

* version_base.h    -- This should be the 1 source for version info.
* version_tag.h     -- Generated automatically with git tag information.
* verilog.spec      -- Used to stamp RPM packages

When versions are changed, the above files need to be edited to account for
the new version information. The following used to have verion information in
them, but now their version information is generated:

Replaced with version_base.h, which is edited manually, and
version_tag.h which is generated from git tag information.

* version-base.in     -- Most compiled code gets version from here

These are now edited by the makefile and the version.exe program.

* iverilog-vpi.man    -- The .TH tag has a version string
* driver/iverilog.man -- The .TH tag has a version string
* driver-vpi/res.rc   -- Used to build Windows version stamp
* vvp/vvp.man         -- The .TH tag has a version string

This now includes version_base.h to get the version

* vpi/vams_simparam.c -- Hard coded result to simulatorVersion query
