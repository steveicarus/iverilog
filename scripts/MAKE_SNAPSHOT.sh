#!/bin/sh

# This script makes snapshots from a git repository. The input is
# the number for a snapshot and the path to a temporary directory.
# for example:
#
#    sh scripts/MAKE_SNAPSHOT.sh 20080428 ~/tmp
#
# The above assumes that there is a tag "s20080428" at the point
# to be snapshot. (The tag has the "s", but the argument to this
# script does not have the "s"). This script extracts based on the
# tag, uses the temporary directory to stage intermediate results,
# and finally creates a file called verilog-20080428.tar.gz that
# contains the snapshot ready to go.
#
# The complete steps to make a snapshot YYYYMMDD generally are:
#
#   edit the verilog.spec to set the rev_date to YYYYMMDD
#
#   git tag -a sYYYYMMDD
#                 (Make the tag in the local git repository.)
#
#   sh scripts/MAKE_SNAPSHOT.sh YYYYMMDD ~/tmp
#                 (Make the snapshot bundle verilog-YYYYMMDD.tar.gz)
#
#   git push --tags
#                 (Publish the tag to the repository.)
#
id=$1
destdir=$2

# The git tag to use is the snapshot id with a prepended "s".
tag="s$id"

# The prefix is the directory that contains the extracted files
# of the bundle. This is also the name of the bundle file itself.
prefix="verilog-$id"

if [ ! -d $destdir ]; then
    echo "ERROR: Directory $destdir does not exist."
    exit 1
fi

if [ -e $destdir/$prefix ]; then
    echo "ERROR: $destdir/$prefix already exists."
    exit 1
fi

echo "Exporting $tag to $destdir/$prefix..."
git archive --prefix="$prefix/" $tag | ( cd "$destdir" && tar xf - )

versionh="$destdir/$prefix/version_tag.h"
echo "Create $versionh ..."
echo "#ifndef VERSION_TAG" > $versionh
echo "#define VERSION_TAG \"$tag\"" >> $versionh
echo "#endif" >> $versionh

echo "Running autoconf.sh..."
( cd $destdir/$prefix && sh autoconf.sh )

echo "Making bundle $prefix.tar.gz..."
tar czf $prefix.tar.gz --exclude=autom4te.cache -C "$destdir" $prefix

echo "Removing temporary $destdir/$prefix..."
rm -rf "$destdir/$prefix"

echo done
