FROM ubuntu:18.04

ARG http_proxy
ARG https_proxy

RUN apt update
RUN apt install -qy wget python3 python3-pip libclang1-6.0
RUN pip3 install clang jinja2
