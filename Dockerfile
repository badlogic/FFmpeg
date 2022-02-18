FROM ubuntu:xenial

ARG DEBIAN_FRONTEND=noninteractive

RUN apt -y update
RUN apt -y install curl wget git \
				   autoconf pkg-config \
			   	   gcc g++ yasm

WORKDIR /code
CMD ./build-linux.sh