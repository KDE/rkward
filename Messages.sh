#!bin/sh
 
# invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources
# the results are stored in a pseudo .cpp file to be picked up by xgettext.
$EXTRACTRC `find rkward -name \*.rc -a \! -name rkward_windows_icon.rc -o -name \*.ui -o -name \*.kcfg`  >> rc.cpp
#
# call xgettext on all source files. If your sources have other filename
# extensions besides .cc, .cpp, and .h, just add them in the find call.
$XGETTEXT `find rkward -name \*.cpp -o -name \*.h -name \*.c` -o $podir/rkward.pot

