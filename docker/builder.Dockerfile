# syntax=docker/dockerfile:1.2
FROM python:bullseye

ARG http_proxy
ARG https_proxy

ARG TARGETARCH

RUN apt update && DEBIAN_FRONTEND=noninteractive apt install -qy libgrpc++-dev protobuf-compiler-grpc

RUN `([ $TARGETARCH = arm64 ] && echo /usr/lib/aarch64-linux-gnu > /tmp/lib ) || ([ $TARGETARCH = amd64 ] && echo /usr/lib/x86_64-linux-gnu > /tmp/lib )`

RUN --mount=type=bind,source=tools/meta-generator,target=/root,rw cd /root && pip install .
RUN --mount=type=bind,target=/root,rw cd /root && make -C meta && cp meta/libmetatai.so `cat /tmp/lib` && cp meta/tai*.h /usr/local/include/
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/framework/examples/basic && cp tools/framework/examples/basic/libtai-basic.so `cat /tmp/lib`/
RUN cd `cat /tmp/lib` && ln -s libtai-basic.so libtai.so
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish && cp tools/taish/taish_server /usr/local/bin/
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish python && cp tools/taish/dist/*.whl /tmp/
