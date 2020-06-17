# syntax=docker/dockerfile:experimental
FROM ubuntu:20.04

ARG http_proxy
ARG https_proxy

RUN --mount=type=cache,target=/var/cache/apt --mount=type=cache,target=/var/lib/apt \
apt update && DEBIAN_FRONTEND=noninteractive apt install -qy libgrpc++-dev g++ protobuf-compiler-grpc make pkg-config python3 python3-pip curl python3-distutils libclang1-6.0 doxygen git

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10
RUN pip install grpcio-tools prompt_toolkit clang jinja2 tabulate grpclib

RUN --mount=type=bind,target=/root,rw cd /root && make && make -C tools/taish
RUN --mount=type=bind,target=/root,rw cd /root && make -C tools/taish python
