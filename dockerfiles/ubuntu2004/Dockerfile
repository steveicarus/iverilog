FROM ubuntu:20.04 as builder

ARG DEBIAN_FRONTEND=noninteractive

# The following will be cached by the docker layer caching mechanism the first time this
# dockerfile is built, and these commands will never execute again unless you force them
# to with the --no-cache option:
# docker build --no-cache . -t iverilog:latest
RUN apt-get -y update && \
    apt-get install -y \
        autoconf \
        automake \
        bison \
        build-essential \
        flex \
        git \
        gperf 

# This clone command results in a bare repo that will remain at the state it
# is at when this Dockerfile is first built and cached by docker. 
# This leverages the caching to avoid repeated full clones, saving bandwidth.
RUN git clone --mirror https://github.com/steveicarus/iverilog iverilog.git

# Bust the cache here or the following 'git remote update' will not execute.
# Pass a random value to the BUST_THE_CAST ENV declared below for 
# every build where you want to bust the cache.  As an alternative to a 
# random variable, you could use a version string or git hash or similar
# to bust the cache only when your (branch, source, whatever) is updated.
# The following build command is a good way to bust the cache here.
# docker build --build-arg IVERILOG_BRANCH=master --build-arg BUST_THE_CACHE=$(date +%s) . -t iverilog:latest
ARG BUST_THE_CACHE="cache not busted"
RUN echo "${BUST_THE_CACHE}" > bust_the_cache.txt
RUN cd iverilog.git && \
    git remote update && \
    cd ..

# The branch to build may be overridden to any git branch or tag 
# using --build-arg on the command line:  
# docker build --build-arg IVERILOG_BRANCH=v10_3 . -t iverilog:10.3
ARG IVERILOG_BRANCH=master

RUN git clone --branch=${IVERILOG_BRANCH} iverilog.git iverilog && \
    cd iverilog && \
    bash autoconf.sh && \
    ./configure && \
    make && \
    make install
    
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update && \
    apt-get install -y \
        make && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local /usr/local/

RUN adduser --disabled-password ic
USER ic
WORKDIR /home/ic

ENTRYPOINT [ "make" ]
