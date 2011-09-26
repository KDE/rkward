#!/bin/bash
cd `dirname $0`/..
BASEDIR=`pwd`

cd ${BASEDIR}/po/
# BZR_HOME=/tmp to achieve anonymous checkout
BZR_HOME=/tmp bzr branch lp:~rkward-devel/rkward/translation-export
cp -a translation-export/po/* .

rm -rf translation-export

svn status
