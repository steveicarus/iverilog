#!/bin/sh

# This script makes a release from a git repository. The input is
# the number for a snapshot and the path to a temporary directory.
# for example:
#
#    sh scripts/MAKE_RELEASE.sh 10.1 ~/tmp
#
# The above assumes that there is a tag "v0_9_1" at the point
# to be released. (The tag has the "v", but the argument to this
# script does not have the "v"). This script extracts based on the
# tag, uses the temporary directory to stage intermediate results,
# and finally creates a file called verilog-0.9.1.tar.gz that
# contains the release ready to go.
#
# The complete steps to make a release x.y generally are:
#
#   Edit version_base.h to suit.
#
#   Edit verilog.spec to suit.
#
#   git tag -a v10_1
#                 (Make the tag in the local git repository.)
#
#   sh scripts/MAKE_RELEASE.sh 10.1 ~/tmp
#                 (Make the snapshot bundle verilog-10.1.tar.gz)
#
#   git push --tags
#                 (Publish the tag to the repository.)
#
id=$1
destdir=$2

# The git tag to use.
tag="v"`echo $id | tr '.' '_'`

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
