# syntax=docker/dockerfile:1.2
FROM docker.io/microsonic/tai-base:latest

ARG http_proxy
ARG https_proxy

ARG TARGETARCH

RUN `([ $TARGETARCH = arm64 ] && echo /usr/lib/aarch64-linux-gnu > /tmp/lib ) || ([ $TARGETARCH = amd64 ] && echo /usr/lib/x86_64-linux-gnu > /tmp/lib )`

RUN --mount=type=bind,target=/root,rw cd /root && cd tools/meta-generator && pip install .
RUN --mount=type=bind,target=/root,rw cd /root && make -C meta && cp meta/libmetatai.so `cat /tmp/lib` && cp meta/tai*.h /usr/local/include/
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/framework/examples/basic && cp tools/framework/examples/basic/libtai.so `cat /tmp/lib`/libtai-basic.so
RUN cd `cat /tmp/lib` && ln -s libtai-basic.so libtai.so
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish && cp tools/taish/taish_server /usr/local/bin/
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish python && cp tools/taish/dist/*.tar.gz /tmp/
