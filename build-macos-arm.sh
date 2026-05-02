#!/bin/bash
# These are pre-installed on GitHub's macOS virtual environment, need to smoke them.
brew install automake autoconf libtool pkg-config curl wget yasm

set -e

export MACOSX_DEPLOYMENT_TARGET=11.0
export PREFIX="${PREFIX:-$PWD/build-macos-arm-prefix}"
export CFLAGS="-arch arm64 -mmacosx-version-min=11.0"
export LDFLAGS="-arch arm64 -mmacosx-version-min=11.0"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/share/pkgconfig"

# build libogg
rm -rf libogg-1.3.5
rm -rf libogg-1.3.5.tar.gz
wget http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
tar xzvf libogg-1.3.5.tar.gz
pushd libogg-1.3.5
./configure --prefix="$PREFIX" --disable-shared
make
make install
popd

# build libvorbis
rm -rf libvorbis-1.3.7
rm -rf libvorbis-1.3.7.tar.gz
wget http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz
tar xzvf libvorbis-1.3.7.tar.gz
pushd libvorbis-1.3.7
./configure --prefix="$PREFIX" --disable-shared
# libvorbis 1.3.7 injects obsolete Darwin linker flag that current Apple ld rejects, especially for arm64.
find . -name Makefile -o -name libtool | xargs sed -i '' 's/-force_cpusubtype_ALL//g'
make -j
make install
popd

# build libvpx
rm -rf libvpx
git clone https://github.com/webmproject/libvpx
pushd libvpx
git checkout v1.11.0
./configure --prefix="$PREFIX" --target=arm64-darwin20-gcc --disable-shared --disable-examples  --disable-tools --disable-unit-tests --disable-decode-perf-tests --disable-encode-perf-tests --extra-cflags="-arch arm64 -mmacosx-version-min=11.0"
make -j
make install
popd

# build libwebp
rm -rf libwebp
git clone https://github.com/webmproject/libwebp
pushd libwebp
./autogen.sh
./configure --prefix="$PREFIX" --disable-gl --disable-sdl --disable-png --disable-jpeg --disable-tiff --disable-gif --disable-wic --disable-shared
# Root make builds command-line examples/tools; current libwebp can fail there on arm64.
# FFmpeg only needs the static libraries, which live under sharpyuv/src.
make -C sharpyuv -j
make -C src -j
make -C sharpyuv install
make -C src install
popd

./configure --fatal-warnings --arch=arm64 --extra-cflags="-arch arm64 -mmacosx-version-min=11.0" --extra-ldflags="-arch arm64 -mmacosx-version-min=11.0 -L$PREFIX/lib" --pkg-config-flags=--static --disable-ffplay --disable-autodetect --enable-zlib \
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
strip ffmpeg ffprobe 2>/dev/null || true
file ffmpeg ffprobe
