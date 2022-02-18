#!/bin/sh
set -e
# brew install libvpx
# Got to delete the following files on macOS otherwise they static libs won't be linked...
rm -f /usr/local/opt/libvpx/lib/*.dylib
rm -f /usr/local/opt/webp/lib/*.dylib
rm -f /usr/local/opt/libx11/lib/*.dylib
rm -f /usr/local/opt/libxcb/lib/*.dylib
rm -f /usr/local/opt/libxau/lib/*.dylib
rm -f /usr/local/opt/libxdmcp/lib/*.dylib

./configure --pkg-config-flags=--static --disable-ffplay \
			--disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages \
			--disable-libxcb --disable-lzma --disable-sdl2 \
			--enable-libvpx --enable-libwebp
make clean
make -j 8