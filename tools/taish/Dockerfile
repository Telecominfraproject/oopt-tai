FROM debian:jessie

RUN apt update
RUN apt install -qy make gcc g++
RUN apt install -qy git
RUN git clone https://github.com/grpc/grpc.git
RUN cd grpc; git submodule update --init --recursive
RUN apt install -qy dh-autoreconf
RUN cd grpc; make
RUN cd grpc; make install
RUN cd grpc/third_party/protobuf; make install
RUN apt install -qy pkg-config
