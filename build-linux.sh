#!/bin/bash
sudo apt update
sudo apt -y install curl wget git \
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
sudo make install
popd

# build libvorbis
rm -rf libvorbis-1.3.7
wget http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz
tar xzvf libvorbis-1.3.7.tar.gz
pushd libvorbis-1.3.7
./configure --disable-shared
make -j
sudo make install
popd

# build libvpx
rm -rf libvpx
git clone https://github.com/webmproject/libvpx
pushd libvpx
git checkout v1.11.0
./configure --disable-shared --disable-examples  --disable-tools --disable-unit-tests --disable-decode-perf-tests --disable-encode-perf-tests
make -j
sudo make install
popd

# build libwebp
rm -rf libwebp
git clone https://github.com/webmproject/libwebp
pushd libwebp
./autogen.sh
./configure --disable-gl --disable-sdl --disable-png --disable-jpeg --disable-tiff --disable-gif --disable-wic --disable-shared
make -j
sudo make install
popd

./configure --logfile=configure.log --extra-ldflags="-static -lpthread" \
			--pkg-config-flags="--static" \
			--fatal-warnings --enable-static --disable-shared --disable-ffplay \
			--disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages \
			--disable-libxcb --disable-lzma --disable-sdl2 \
			--disable-avdevice --disable-postproc \
			--disable-protocols --disable-indevs --disable-outdevs --disable-devices \
			--disable-encoders --disable-decoders --disable-muxers --disable-demuxers --disable-protocols --disable-filters \
			--enable-encoder=mjpeg --enable-decoder=mjpeg \
			--enable-encoder=mpeg4 --enable-decoder=mpeg4 \
			--enable-encoder=png --enable-decoder=png \
			--enable-encoder=prores_ks --enable-decoder=prores \
			--enable-encoder=libwebp_anim \
			--enable-encoder=libwebp --enable-decoder=webp \
			--enable-encoder=libvpx_vp9 --enable-decoder=libvpx_vp9 \
			--enable-encoder=ac3 --enable-decoder=ac3 \
			--enable-encoder=aac --enable-decoder=aac \
			--enable-encoder=pcm_s16le --enable-decoder=pcm_s16le \
			--enable-encoder=libvorbis --enable-decoder=libvorbis \
			--enable-encoder=rawvideo --enable-decoder=rawvideo \
			--enable-muxer=avi --enable-demuxer=avi \
			--enable-muxer=mov --enable-demuxer=mov \
			--enable-muxer=webp --enable-demuxer=image_webp_pipe \
			--enable-muxer=webm --enable-demuxer=webm_dash_manifest \
			--enable-muxer=rawvideo --enable-demuxer=rawvideo \
			--enable-muxer=pcm_s16le --enable-demuxer=pcm_s16le \
			--enable-muxer=image2 \
			--enable-protocol=file --enable-protocol=tcp \
			--enable-filter=scale --enable-filter=select --enable-filter=aresample \
			--enable-libvpx --enable-libwebp --enable-libvorbis
make clean
make -j