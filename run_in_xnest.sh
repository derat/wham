#!/bin/sh
Xnest -ac -geometry 1000x720 :1 &
export DISPLAY=:1
xsetroot -solid black
./wham
