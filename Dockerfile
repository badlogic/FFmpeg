FROM debian:stretch-slim

RUN apt-get update && apt-get -y install curl gcc g++ gcc-multilib g++-multilib mingw-w64 lib32z1 git

WORKDIR /code/jni
CMD echo "ready"