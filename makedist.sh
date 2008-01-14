#!/bin/bash

VERSION=${1}
BASEDIR=`pwd`
mkdir $BASEDIR/disttemp
DISTDIRREL=rkward-$VERSION
DISTDIR=$BASEDIR/disttemp/$DISTDIRREL
mkdir $DISTDIR

#prepare version.h
echo "/* Version number of package */" > $DISTDIR/version.h
echo "#define VERSION \"$VERSION\"" >> $DISTDIR/version.h

cp -a AUTHORS CMakeLists.txt COPYING ChangeLog TODO INSTALL NOTES README configure $DISTDIR
mkdir $DISTDIR/doc
mkdir $DISTDIR/po
mkdir $DISTDIR/rkward

rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/doc/* $DISTDIR/doc
rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/po/* $DISTDIR/po
rsync -a --exclude '*~' --exclude '*.svn*' --exclude 'templates' $EXCLUDES $BASEDIR/rkward/* $DISTDIR/rkward

cd $DISTDIR/po
./Messages.sh
cd $BASEDIR/disttemp

tar -czf rkward-$VERSION.tar.gz $DISTDIRREL
mv rkward-$VERSION.tar.gz $BASEDIR/

cd $BASEDIR
rm -rf $BASEDIR/disttemp
