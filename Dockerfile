ARG IMAGE="alpine:3.12"
# Note that some commands such as 'apk add' won't work in arbitrary images.  For instance,
# 'apk add' works fine under alpine, but not under Ubuntu (at least not by default.)
# In other words, the following 'apk add' sections assume an alpine based image or a derivitive

FROM $IMAGE as base

RUN apk add --no-cache \
        bzip2 \
        libbz2 \
        libgcc \
        libhistory \
        libstdc++ \
        readline \
        zlib

FROM base as builder

RUN apk add --no-cache \
        autoconf \
        build-base \
        bison \
        bzip2-dev \
        flex \
        git \
	gperf \
        readline-dev \
        zlib-dev

COPY . .

RUN sh autoconf.sh && \
    ./configure && \
    make && \
    make install

FROM builder as builder-iverilog-regression-test

RUN make check

ARG REGRESSION_TEST_URL=https://github.com/steveicarus/ivtest.git
RUN git clone ${REGRESSION_TEST_URL} ivtest

# Running the tests here was useful for troubleshooting, but we also run them below
# in a lighter weight image so it is not necessary to run them here anymore
# WORKDIR ivtest
# RUN perl vvp_reg.pl
# RUN perl vpi_reg.pl

FROM base as release-candidate

COPY --from=builder /usr/local /usr/local/

FROM release-candidate as iverilog-vpi

RUN apk add --no-cache \
     build-base

FROM iverilog-vpi as test-iverilog-vpi

RUN apk add --no-cache \
     perl

COPY --from=builder-iverilog-regression-test /ivtest /ivtest

WORKDIR /ivtest
RUN perl vvp_reg.pl
RUN perl vpi_reg.pl

FROM release-candidate as iverilog
#
# Below are some sample commands to build docker images.
# 
# The vpi_reg.pl script wont run in this 87.5 MB image which does not contain perl or c/c++
# docker build  . -t iverilog
#
# This is a larger 298 MB image with c/c++ compilers through build-base
# docker build --target iverilog-vpi . -t iverilog-vpi
#
# This is a larger 343 MB image with c/c++ compilers throubh build-base and perl
# docker build --target test-iverilog-vpi . -t iverilog-perl
#
# This is a larger 598 MB image with full featured compiler, git, and full build results 
# docker build --target builder . -t iverilog-builder