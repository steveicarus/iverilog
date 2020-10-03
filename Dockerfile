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
        make \
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

FROM builder as test-builder

RUN make check

FROM base as test-release-candidate

COPY --from=builder /usr/local /usr/local/

FROM test-release-candidate as release-candidate

RUN adduser --disabled-password ic
USER ic
WORKDIR /home/ic

FROM release-candidate as release-candidate-entrypoint-make

ENTRYPOINT [ "make" ]

# This commented section or something similar may be used to test the release candidate
# image before it is finally released.  A failure here would stop the process so that
# a faulty image is not released.
#
# We create a layer that contains the tests as builder-iverilog-regression-test here:

FROM builder as builder-iverilog-regression-test-base

ARG REGRESSION_TEST_URL=https://github.com/steveicarus/ivtest.git
RUN git clone ${REGRESSION_TEST_URL} ivtest

FROM builder-iverilog-regression-test-base as builder-iverilog-regression-test

WORKDIR ivtest
RUN perl vvp_reg.pl
RUN perl vpi_reg.pl

FROM test-release-candidate as test-release-candidate-perl

RUN apk add --no-cache \
    musl \
    perl

RUN adduser --disabled-password ic
USER ic
WORKDIR /home/ic

COPY --from=builder-iverilog-regression-test-base /ivtest /home/ic/

RUN perl vvp_reg.pl
# RUN perl vpi_reg.pl
# RUN perl vhdl_reg.pl

FROM release-candidate-entrypoint-make as iverilog-make

FROM release-candidate as iverilog

# Below are some sample commands to build docker images.
# 
# docker build  . -t iverilog
#
# docker build --target iverilog-make . -t iverilog-make