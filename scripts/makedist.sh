#!/bin/bash

cd `dirname $0`/..
BASEDIR=`pwd`
OLDVERSION=`${BASEDIR}/scripts/getversion.sh`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`

# import the translations
$BASEDIR/scripts/import_translations.py
$BASEDIR/scripts/check_translations.py

mkdir $BASEDIR/disttemp
DISTDIRREL=rkward-$VERSION
DISTDIR=$BASEDIR/disttemp/$DISTDIRREL
mkdir $DISTDIR

$BASEDIR/scripts/set_dist_version.sh $VERSION

# update roxygen documentation just in case we forgot:
$BASEDIR/scripts/roxygenize.sh || exit 1

cp -a AUTHORS CMakeLists.txt COPYING ChangeLog TODO INSTALL NOTES README configure VERSION.cmake $DISTDIR
mkdir $DISTDIR/doc
mkdir $DISTDIR/i18n
mkdir $DISTDIR/po
mkdir $DISTDIR/rkward
mkdir $DISTDIR/tests

rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/doc/* $DISTDIR/doc
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/i18n/* $DISTDIR/i18n
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/po/* $DISTDIR/po
rsync -a --exclude '*~' --exclude '*.git*' --exclude 'templates' --exclude 'rbackend/rpackages/rkwardtests/debian' $EXCLUDES $BASEDIR/rkward/* $DISTDIR/rkward
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/tests/* $DISTDIR/tests

cd $BASEDIR/disttemp
tar -czf rkward-$VERSION.tar.gz $DISTDIRREL
mv rkward-$VERSION.tar.gz $BASEDIR/

cd $BASEDIR
rm -rf $BASEDIR/disttemp

$BASEDIR/scripts/set_dist_version.sh $OLDVERSION
