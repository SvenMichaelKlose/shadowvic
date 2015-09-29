#!/bin/sh

echo "Compiling…"
cd src; cc $@ -O3 -Wall -o ../shadowvic 6502.c 6561.c disassembler.c fb.c joystick.c main.c sync.c vic-20.c video.c; cd ..
