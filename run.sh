#!/bin/sh
cd build
./xdm -nodaemon -debug 3 -server ":4 XDMServer local /usr/bin/Xnest :4" -config /home/midenok/src/xdm-1.1.10-modified/xdm-config-silent
