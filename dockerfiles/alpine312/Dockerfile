FROM alpine:3.12 as builder

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

# The branch to build may be overridden to any git branch or tag 
# using --build-arg on the command line:  
# docker build --build-arg IVERILOG_BRANCH=v10_3 . -t iverilog:10.3
ARG IVERILOG_BRANCH=master

RUN git clone --branch=${IVERILOG_BRANCH} https://github.com/steveicarus/iverilog && \
    cd iverilog && \
    sh autoconf.sh && \
    ./configure && \
    make -j4 && \
    make install && \
    cd && \
    rm -rf iverilog

FROM alpine:3.12

RUN apk add --no-cache \
        bzip2 \
        libbz2 \
        libgcc \
        libhistory \
        libstdc++ \
        make \
        readline \
        zlib

COPY --from=builder /usr/local /usr/local/

RUN adduser --disabled-password ic
USER ic
WORKDIR /home/ic

ENTRYPOINT [ "make" ]
