#!/bin/sh
Xephyr :1 -ac -br -dpi 105 -screen 1000x720 &
export DISPLAY=:1
sleep 3
if [ -e $HOME/.Xresources ]; then xrdb -load $HOME/.Xresources; fi
./wham
