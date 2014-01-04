
# This is a little developer convenience script to run the ivl core program
# in place with the stub target. It runs the ivl core verbose, with diagnostic
# output files enable, and without the driver program or preprocessor.
# It is useful only for development of the ivl core program.
#
# Run this script in the source directory for the ivl core program so that
# the patch to the other components is correct.
#
# NOTE: DO NOT INSTALL THIS FILE.

./ivl -v -Ctgt-stub/stub.conf -C./scripts/devel-stub.conf -Pa.pf -Na.net -fDLL=tgt-stub/stub.tgt foo.vl | tee foo.log 2>&1

echo "*** ivl command completed"
