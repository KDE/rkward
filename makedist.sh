#!/bin/bash

VERSION=${1}
cd `dirname $0`
BASEDIR=`pwd`
mkdir $BASEDIR/disttemp
DISTDIRREL=rkward-$VERSION
DISTDIR=$BASEDIR/disttemp/$DISTDIRREL
mkdir $DISTDIR

$BASEDIR/set_dist_version.sh $VERSION

cp -a AUTHORS CMakeLists.txt COPYING ChangeLog TODO INSTALL NOTES README configure $DISTDIR
mkdir $DISTDIR/doc
mkdir $DISTDIR/po
mkdir $DISTDIR/rkward
mkdir $DISTDIR/tests

rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/doc/* $DISTDIR/doc
rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/po/* $DISTDIR/po
rsync -a --exclude '*~' --exclude '*.svn*' --exclude 'templates' $EXCLUDES $BASEDIR/rkward/* $DISTDIR/rkward
rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/tests/* $DISTDIR/tests

cd $DISTDIR/po
./Messages.sh
cd $BASEDIR/disttemp

tar -czf rkward-$VERSION.tar.gz $DISTDIRREL
mv rkward-$VERSION.tar.gz $BASEDIR/

cd $BASEDIR
rm -rf $BASEDIR/disttemp
