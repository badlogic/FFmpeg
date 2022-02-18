#!/bin/bash
apt update
apt -y install curl wget git \
			   autoconf make pkg-config libtool \
			   gcc g++ yasm

set -e

# build libogg
rm -rf libogg-1.3.5
wget http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
tar xzvf libogg-1.3.5.tar.gz
pushd libogg-1.3.5
./configure --disable-shared
make
make install
popd

# build libvorbis
rm -rf libvorbis-1.3.7
wget http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz
tar xzvf libvorbis-1.3.7.tar.gz
pushd libvorbis-1.3.7
./configure --disable-shared
make -j
make install
popd

# build libvpx
rm -rf libvpx
git clone https://github.com/webmproject/libvpx
pushd libvpx
git checkout v1.11.0
./configure --disable-shared --disable-examples  --disable-tools --disable-unit-tests --disable-decode-perf-tests --disable-encode-perf-tests
make -j
make install
popd

# build libwebp
rm -rf libwebp
git clone https://github.com/webmproject/libwebp
pushd libwebp
./autogen.sh
./configure --disable-gl --disable-sdl --disable-png --disable-jpeg --disable-tiff --disable-gif --disable-wic --disable-shared
make -j
make install
popd

./configure --logfile=configure.log --extra-ldflags="-static -lpthread -lm" \
			--pkg-config-flags="--static" \
			--fatal-warnings --enable-static --disable-shared --disable-ffplay \
			--disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages \
			--disable-libxcb --disable-lzma --disable-sdl2 \
			--enable-libwebp --enable-libvpx
make clean
make -j