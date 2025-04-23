#!/bin/bash
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#

cd `dirname $0`/..
BASEDIR=`pwd`
OLDVERSION=`${BASEDIR}/scripts/getversion.sh`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`

# import the translations
$BASEDIR/scripts/check_translations.py --strict

mkdir $BASEDIR/disttemp
DISTDIRREL=rkward-$VERSION
DISTDIR=$BASEDIR/disttemp/$DISTDIRREL
mkdir $DISTDIR

$BASEDIR/scripts/set_dist_version.sh $VERSION

# update roxygen documentation just in case we forgot:
$BASEDIR/scripts/roxygenize.sh || exit 1

cp -a AUTHORS CMakeLists.txt COPYING ChangeLog TODO INSTALL NOTES README configure VERSION.cmake $DISTDIR
mkdir $DISTDIR/doc
mkdir $DISTDIR/po
mkdir $DISTDIR/rkward
mkdir $DISTDIR/tests
mkdir $DISTDIR/3rdparty
mkdir $DISTDIR/LICENSES

rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/doc/* $DISTDIR/doc
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/po/* $DISTDIR/po
rsync -a --exclude '*~' --exclude '*.git*' --exclude 'templates' --exclude 'rbackend/rpackages/rkwardtests/debian' $EXCLUDES $BASEDIR/rkward/* $DISTDIR/rkward
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/tests/* $DISTDIR/tests
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/3rdparty/* $DISTDIR/3rdparty
rsync -a --exclude '*~' --exclude '*.git*' $EXCLUDES $BASEDIR/LICENSES/* $DISTDIR/LICENSES

cd $BASEDIR/disttemp
tar -czf rkward-$VERSION.tar.gz $DISTDIRREL
mv rkward-$VERSION.tar.gz $BASEDIR/

cd $BASEDIR
rm -rf $BASEDIR/disttemp

$BASEDIR/scripts/set_dist_version.sh $OLDVERSION
