FROM debian:jessie

RUN apt update && apt install -qy make gcc g++ git dh-autoreconf pkg-config
RUN git clone https://github.com/grpc/grpc.git && cd grpc; git submodule update --init --recursive; make install; cd third_party/protobuf; make install
