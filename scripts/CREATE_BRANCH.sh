#!/bin/sh

# This script creates a new release branch using the major version number
# that is passed as the only argument,
#
# 1. It creates a new branch with the proper name.
#
# 2. It then updates the version_base.h to match this version. It likely
#    already does, but it is incorrectly marked as devel instead of stable.
#
# 3. It updates the default suffix in aclocal.m4 to match the branch.
#
# Now manually push the new branch to the master repository.
#
#     git push -u origin v13-branch
#
# After this update the master branch to the next version and update
# vvp/examples/hello.vvp to match the new version.
#

if [ $# -ne 1 ] ; then
    echo "Usage: CREATE_BRANCH.sh <major-number>"
    exit 1;
fi
case $1 in
  *[!0-9]*) echo "Major number must be numeric"; exit 1;;
esac

major=$1

branch="v${major}-branch"

branch_exists=`git ls-remote --heads origin $branch`
if [ -n "$branch_exists" ] ; then
    echo "The branch $branch already exists. Aborting"
    exit 1
fi

echo "Creating branch $branch"
git checkout -b $branch

echo "Updating version_base.h..."
sed -i -E "s/(define\s+VERSION_MAJOR\s+).*/\1$major/" version_base.h
sed -i -E "s/(define\s+VERSION_MINOR\s+).*/\10/" version_base.h
sed -i -E "s/(define\s+VERSION_EXTRA\s+).*/\1\" \(stable\)\"/" version_base.h

echo "Updating aclocal.m4..."
sed -i -E "s/(install_suffix='-)dev/\1$major/" aclocal.m4

echo "Adding updated files to the new branch..."
git add version_base.h aclocal.m4
git commit -m "Creating new branch $branch"

echo "Done"
