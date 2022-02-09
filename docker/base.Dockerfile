# syntax=docker/dockerfile:1.2
FROM ubuntu:20.04

ARG http_proxy
ARG https_proxy

ARG TARGETARCH

RUN apt update && DEBIAN_FRONTEND=noninteractive apt install -qy libgrpc++-dev g++ protobuf-compiler-grpc make pkg-config python3 python3-pip curl python3-distutils libclang1-6.0 doxygen git

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10
RUN --mount=type=cache,target=/root/.cache,sharing=private pip install grpcio grpcio-tools prompt_toolkit tabulate
