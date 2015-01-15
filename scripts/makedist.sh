#!/bin/bash

cd `dirname $0`/..
BASEDIR=`pwd`
OLDVERSION=`${BASEDIR}/scripts/getversion.sh`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`

if [ ! -d "$BASEDIR/i18n/po" ]; then
  echo "Translations are missing!"
  exit 1
fi

mkdir $BASEDIR/disttemp
DISTDIRREL=rkward-$VERSION
DISTDIR=$BASEDIR/disttemp/$DISTDIRREL
mkdir $DISTDIR

$BASEDIR/scripts/set_dist_version.sh $VERSION

# update roxygen documentation just in case we forgot:
$BASEDIR/scripts/roxygenize.sh

cp -a AUTHORS CMakeLists.txt COPYING ChangeLog TODO INSTALL NOTES README configure VERSION.cmake $DISTDIR
mkdir $DISTDIR/doc
mkdir $DISTDIR/i18n
mkdir $DISTDIR/rkward
mkdir $DISTDIR/tests

rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/doc/* $DISTDIR/doc
rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/i18n/* $DISTDIR/i18n
rsync -a --exclude '*~' --exclude '*.svn*' --exclude 'templates' $EXCLUDES $BASEDIR/rkward/* $DISTDIR/rkward
rsync -a --exclude '*~' --exclude '*.svn*' $EXCLUDES $BASEDIR/tests/* $DISTDIR/tests

# make messages
#cd $DISTDIR/po
#./Messages.sh
# TODO: Import translations

cd $BASEDIR/disttemp
tar -czf rkward-$VERSION.tar.gz $DISTDIRREL
mv rkward-$VERSION.tar.gz $BASEDIR/

cd $BASEDIR
rm -rf $BASEDIR/disttemp

$BASEDIR/scripts/set_dist_version.sh $OLDVERSION
