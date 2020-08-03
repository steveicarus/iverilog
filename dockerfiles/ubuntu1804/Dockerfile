FROM ubuntu:18.04 as builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update && \
    apt-get install -y \
        autoconf \
        automake \
        bison \
        build-essential \
        flex \
        git \
        gperf && \
    rm -rf /var/lib/apt/lists/*

ENV IVERILOG_VERSION=v10_3

RUN git clone --branch=${IVERILOG_VERSION} https://github.com/steveicarus/iverilog && \
    cd iverilog && \
    bash autoconf.sh && \
    ./configure && \
    make && \
    make install && \
    cd && \
    rm -rf iverilog

FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update && \
    apt-get install -y \
        make && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local /usr/local/

WORKDIR /root/iv
ENTRYPOINT [ "make" ]



