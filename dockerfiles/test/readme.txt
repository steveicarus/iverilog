The following presumes you have built the docker container and 
tagged it with an iverilog:10.3 release tag.  Use the 
'docker image ls' command to list the images on your machine,
and determine the name of your particular image.
   
The following command runs Make within the Docker container on 
the Makefile in the working (current) directory, which is mapped 
through the volume -v parameter to /home/ic inside the container:

docker run -v $(pwd):/home/ic iverilog:10.3

The Dockerfile entrypoint runs make, with an optional command 
passed as an argument.  To run 'make clean' on Makefile, 
add 'clean' to the end of the docker run command:

docker run -v $(pwd):/home/ic iverilog:10.3 clean

Add a Makefile to your own project's working directory and 
modify the Makefile as you see fit.  The Makefile executes 
the icarus verilog tools inside the container, and stores
the results in the container's /home/ic directory which 
is volume mapped to the working directory containing the 
Makefile by the docker run -v command. 

