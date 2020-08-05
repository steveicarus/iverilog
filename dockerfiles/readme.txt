To build this Dockerfile with the latest code in master, from
its location within the alpine312 subdirectory:
cd alpine312
docker build . -t iverilog

If you prefer, you can tag it with the release number, or latest:
docker build . -t iverilog:latest

The line in the Dockerfile that sets the default branch to build is: 
ARG IVERILOG_BRANCH=master

To build this Dockerfile with the code in the v10_3 tagged release, use 
--build-arg to override IVERILOG_BRANCH to pull the v10_3 tag with:
docker build --build-arg IVERILOG_BRANCH=v10_3 . -t iverilog:10.3

This docker container's entrypoint runs make, with an optional command passed 
as an argument at the end of the command line.  See the readme.txt file
in the test subdirectory for an example, and instructions on using 
a Makefile to run icarus tools within the container.

