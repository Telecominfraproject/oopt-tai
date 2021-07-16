# syntax=docker/dockerfile:1.2
FROM ubuntu:20.04

ARG http_proxy
ARG https_proxy

ARG TARGETARCH

RUN rm -f /etc/apt/apt.conf.d/docker-clean; echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache
RUN --mount=type=cache,target=/var/cache/apt,sharing=private --mount=type=cache,target=/var/lib/apt,sharing=private \
apt update && DEBIAN_FRONTEND=noninteractive apt install -qy libgrpc++-dev g++ protobuf-compiler-grpc make pkg-config python3 python3-pip curl python3-distutils libclang1-6.0 doxygen git

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10
RUN --mount=type=cache,target=/root/.cache,sharing=private pip install grpcio-tools prompt_toolkit clang jinja2 tabulate grpclib

RUN `([ $TARGETARCH = arm64 ] && echo /usr/lib/aarch64-linux-gnu > /tmp/lib ) || ([ $TARGETARCH = amd64 ] && echo /usr/lib/x86_64-linux-gnu > /tmp/lib )`
RUN --mount=type=bind,target=/root,rw cd /root && make -C meta && cp meta/libmetatai.so `cat /tmp/lib` && cp meta/tai*.h /usr/local/include/
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/framework/examples/basic && cp tools/framework/examples/basic/libtai.so `cat /tmp/lib`/libtai-basic.so
RUN cd `cat /tmp/lib` && ln -s libtai-basic.so libtai.so
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish && cp tools/taish/taish_server /usr/local/bin/
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish python && cp tools/taish/dist/*.tar.gz /tmp/
