#!/bin/sh

# This script prepares and tags a release in the git repository. The tag is
# based on the first and second argument passed to the script, which should
# be the desired major and minor numbers for the release. Before creating the
# tag, autoconf.sh will be run to create the configure and lexor_keyword.cc
# files, the version_base.h and verilog.spec files will be updated to reflect
# the new release ID and a release_tag.h file will be created in the top level
# directory to provide the VERSION_TAG macro. After creating the tag, the
# configure, lexor_keywords.cc, and release_tag.h files will be deleted.
#
# The complete steps to publish a release are:
#
#   sh scripts/CREATE_RELEASE.sh
#                 (Create the tag and version info in the local repository)
#
#   git push --follow-tags
#                 (Push the commits and tag to the remote repository.)
#
# For a major release, create and switch to the new release branch before
# running this script.

if [ $# -ne 2 ] ; then
    echo "Usage: CREATE_RELEASE.sh <major-number> <minor-number>"
    exit 1;
fi
case $1 in
  *[!0-9]*) echo "Major number must be numeric"; exit 1;;
esac
case $2 in
  *[!0-9]*) echo "Minor number must be numeric"; exit 1;;
esac

major=$1
minor=$2

date=`date +%Y%m%d`

tag="v${major}_${minor}"

tag_exists=`git tag -l $tag`
if [ -n "$tag_exists" ] ; then
    echo "The tag $tag already exists. Aborting"
    exit 1
fi

echo "Executing autoconf.sh..."
sh autoconf.sh
if [ $? -ne 0 ] ; then
    echo "autoconf.sh failed"
    exit 1
fi

echo "Updating version_base.h..."
sed -i -E "s/(define\s+VERSION_MAJOR\s+).*/\1$major/" version_base.h
sed -i -E "s/(define\s+VERSION_MINOR\s+).*/\1$minor/" version_base.h
sed -i -E "s/(define\s+VERSION_EXTRA\s+).*/\1\" \(stable\)\"/" version_base.h

echo "Updating verilog.spec..."
sed -i -E "s/(define\s+major\s+).*/\1$major/" verilog.spec
sed -i -E "s/(define\s+minor\s+).*/\1$minor/" verilog.spec
sed -i -E "s/(define\s+rev_date\s+).*/\1$date/" verilog.spec

echo "Creating release_tag.h..."
echo "#define VERSION_TAG \"$tag\"" > release_tag.h

echo "Adding files and creating the tag..."
git add -f configure lexor_keyword.cc vhdlpp/lexor_keyword.cc
git add version_base.h verilog.spec release_tag.h
git commit -m "Creating release $tag"
git tag -a -m "Release $major.$minor" $tag

echo "Deleting temporary files..."
git rm --cached configure lexor_keyword.cc vhdlpp/lexor_keyword.cc
git rm release_tag.h
git commit -m "Post-release cleanup"

echo "Done"
