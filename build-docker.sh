#!/bin/bash
docker build -t ffmpeg-docker . && docker run --rm -v "$(pwd):/code" -it ffmpeg-docker