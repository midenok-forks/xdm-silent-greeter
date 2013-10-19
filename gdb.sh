#!/bin/sh
cd build
gdb --args ./xdm -nodaemon -debug 10 -server ":4 XDMServer local /usr/bin/Xnest :4" -config /home/midenok/src/xdm-1.1.10-modified/xdm-config
