# syntax=docker/dockerfile:experimental
ARG TAI_DOCKER_BUILDER_IMAGE=tai-builder:latest

FROM ${TAI_DOCKER_BUILDER_IMAGE} as builder

FROM ubuntu:20.04

ARG http_proxy
ARG https_proxy

RUN --mount=type=cache,target=/var/cache/apt --mount=type=cache,target=/var/lib/apt \
apt update && DEBIAN_FRONTEND=noninteractive apt install -qy --no-install-recommends python3 python3-pip make libgrpc++1

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10
RUN --mount=type=bind,source=/tmp,target=/tmp,from=builder pip install /tmp/*.tar.gz
RUN --mount=type=bind,target=/tmp,from=builder cp /tmp/usr/lib/x86_64-linux-gnu/libtai-basic.so /usr/lib/x86_64-linux-gnu/
RUN cd /usr/lib/x86_64-linux-gnu && ln -s libtai-basic.so libtai.so

RUN --mount=type=bind,target=/tmp,from=builder cp /tmp/usr/lib/x86_64-linux-gnu/libmetatai.so /usr/lib/x86_64-linux-gnu/
RUN --mount=type=bind,target=/tmp,from=builder cp /tmp/usr/local/bin/taish_server /usr/local/bin/
