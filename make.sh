#!/bin/sh

set -e

echo "Compiling…"
cd src; cc $@ -O3 -Wall -I../include -o ../shadowvic 6502.c 6561.c disassembler.c fb.c joystick.c main.c sync.c vic-20.c video.c; cd ..
cd src; cc $@ -O3 -Wall -flto -c -I../include 6502.c 6561.c disassembler.c fb.c joystick.c sync.c vic-20.c video.c; cd ..
ar cru -o libshadowvic.a src/*.o
