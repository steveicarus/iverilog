#!/bin/sh

# This script manually creates a version.h file.
#
# It is used when creating a MinGW executable from a Cygwin
# hosted git repository. It assumes that git is available.
#
#    sh scripts/CREATE_VERSION.sh
#

echo "Building version_tag.h with git describe"
tmp=`git describe | sed -e 's;\(.*\);#define VERSION_TAG "\1";'`
echo "#ifndef VERSION_TAG" > version_tag.h
echo "$tmp" >> version_tag.h
echo "#endif" >> version_tag.h
