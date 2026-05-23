#!/bin/sh

# This script creates a new release branch using the major version number
# that is passed as the only argument,
#
# 1. It creates a new branch with the proper name.
#
# 2. It then updates the configure.ac to match this version. It likely
#    already does, but it is incorrectly marked as devel instead of stable.
#
# 3. It updates the default suffix in m4/ax_enable_suffix.m4 to match the branch.
#
# Now manually push the new branch to the master repository.
#
#     git push -u origin v13-branch
#
# After this update the master branch to the next version and update
# vvp/examples/*.vvp to match the new version.
#

if [ $# -ne 1 ] ; then
    echo "Usage: CREATE_BRANCH.sh <major-number>"
    exit 1;
fi
case $1 in
  *[!0-9]*) echo "Major number must be numeric"; exit 1;;
esac

major=$1
minor=0
extra="stable"
branch="v${major}-branch"

branch_exists=`git ls-remote --heads origin $branch`
if [ -n "$branch_exists" ] ; then
    echo "The branch $branch already exists. Aborting"
    exit 1
fi

echo "Creating branch $branch"
git checkout -b $branch

file=configure.ac
echo "Updating $file..."
sed -i -E "s/(m4_define\(\[VER_MAJOR\],[[:space:]]*\[)[^]]*(\]\))/\1$major\2/" $file
sed -i -E "s/(m4_define\(\[VER_MINOR\],[[:space:]]*\[)[^]]*(\]\))/\1$minor\2/" $file
sed -i -E "s/(m4_define\(\[VER_EXTRA\],[[:space:]]*\[)[^]]*(\]\))/\1$extra\2/" $file

echo "Updating m4/ax_enable_suffix.m4..."
sed -i -E "s/(install_suffix='-)dev/\1$major/" m4/ax_enable_suffix.m4

echo "Adding updated files to the new branch..."
git add configure.ac m4/ax_enable_suffix.m4
git commit -m "Creating new branch $branch"

echo "Done"
