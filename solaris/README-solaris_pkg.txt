Notes about the solaris package.

I.  Installing a prebuilt solaris package
-----------------------------------------

To install the solaris package do the following as root on your machine:

1) uncompress the package using:

     cp package_name.gz /tmp
     cd /tmp
     gunzip package_name.gz

   where  "package_name.gz" is the compressed binary package.  It will
   be named something like "verilog-0.3-SunOS-5.6-sparc.gz"

2) install the package using:

     cd /tmp
     pkgadd -d package_name

   this will install the package.  The package will be registered under the
   name "IVLver"

II.  Uninstalling the solaris package
-------------------------------------

To uninstall an installed solaris package do the following as root on your machine:

    pkgrm IVLver

III.  Notes on building a solaris package from sources
------------------------------------------------------

1) build and install verilog.  Be sure and pick where the package should
   install with the "--prefix=" argument to 'configure'

2) edit the 'pkginfo' file to update the version number and also set BASEDIR
   to the same as the argument to "--prefix="

3) edit the 'prototype' file to add/removed files/directories from the list
   of installed components.

4) run the 'mksolpkg' script to create the solaris package

