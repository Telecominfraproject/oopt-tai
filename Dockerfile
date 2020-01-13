FROM ubuntu:19.04

ARG http_proxy
ARG https_proxy

RUN apt update && apt install -qy libgrpc++-dev g++ protobuf-compiler-grpc make pkg-config python3 curl python3-distutils libclang1-6.0 doxygen git
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN curl -kL https://bootstrap.pypa.io/get-pip.py | python
RUN pip install grpcio-tools prompt_toolkit clang jinja2 tabulate grpclib
