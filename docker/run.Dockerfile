# syntax=docker/dockerfile:1.2
ARG TAI_DOCKER_BUILDER_IMAGE=tai-builder:latest

FROM ${TAI_DOCKER_BUILDER_IMAGE} as builder

FROM ubuntu:20.04

ARG http_proxy
ARG https_proxy

RUN rm -f /etc/apt/apt.conf.d/docker-clean; echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache
RUN --mount=type=cache,target=/var/cache/apt,sharing=private --mount=type=cache,target=/var/lib/apt,sharing=private \
apt update && DEBIAN_FRONTEND=noninteractive apt install -qy --no-install-recommends python3 python3-pip make libgrpc++1

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10
RUN --mount=type=bind,source=/tmp,target=/tmp,from=builder pip install /tmp/*.tar.gz
RUN --mount=type=bind,target=/tmp,from=builder cp /tmp/`cat /tmp/tmp/lib`/libtai-basic.so `cat /tmp/tmp/lib`
RUN cd `cat /tmp/tmp/lib` && ln -s libtai-basic.so libtai.so

RUN --mount=type=bind,target=/tmp,from=builder cp /tmp/`cat /tmp/tmp/lib`/libmetatai.so `cat /tmp/tmp/lib`
RUN --mount=type=bind,target=/tmp,from=builder cp /tmp/usr/local/bin/taish_server /usr/local/bin/
