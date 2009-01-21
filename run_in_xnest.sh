#!/bin/sh
Xnest -ac -geometry 1000x720 :1 &
export DISPLAY=:1
while ! xsetroot -solid black; do sleep 1; done
xrdb -load $HOME/.Xresources
./wham
