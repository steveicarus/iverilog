#!/bin/sh

# This script prepares and tags a snapshot in the git repository. The tag is
# based on the current date, e.g. when created on 03-Jan-2025 the tag will be
# s20250103. Before creating the tag, the verilog.spec file will be updated
# to reflect the snapshot date and a release_tag.h file will be created in
# the top level directory to provide the VERSION_TAG macro. After creating
# the tag, the release_tag.h file will be deleted.
#
# The complete steps to publish a snapshot are:
#
#   sh scripts/CREATE_SNAPSHOT.sh
#                 (Create the tag and version info in the local repository)
#
#   git push --follow-tags
#                 (Push the commits and tag to the remote repository.)

date=`date +%Y%m%d`

tag="s$date"

tag_exists=`git tag -l $tag`
if [ -n "$tag_exists" ] ; then
    echo "The tag $tag already exists. Aborting"
    exit 1
fi

echo "Updating verilog.spec..."
sed -i -E "s/(define\s+rev_date\s+).*/\1$date/" verilog.spec

echo "Creating release_tag.h..."
echo "#define VERSION_TAG \"$tag\"" > release_tag.h

echo "Adding modified files and creating the tag..."
git add verilog.spec release_tag.h
git commit -m "Creating snapshot $tag"
git tag -a -m "Snapshot $id" $tag

echo "Deleting release_tag.h..."
git rm release_tag.h
git commit -m "Post-snapshot cleanup"

echo "Done"