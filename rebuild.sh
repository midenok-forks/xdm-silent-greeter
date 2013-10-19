#!/bin/sh
rm stampdir/config stampdir/build
DEB_BUILD_OPTIONS=noopt debian/rules build
