#!/bin/bash
# create multiresolution windows icon
ICON_DST=../../src/qt/res/icons/artboomerang.ico

convert ../../src/qt/res/icons/artboomerang-16.png ../../src/qt/res/icons/artboomerang-32.png ../../src/qt/res/icons/artboomerang-48.png ${ICON_DST}
