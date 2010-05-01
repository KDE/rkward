#!/bin/bash
#
# Update the version information in the various places around the source
#

VERSION=${1}
cd `dirname $0`/..
BASEDIR=`pwd`

echo "/* Version number of package */" > $BASEDIR/rkward/version.h
echo "#define VERSION \"$VERSION\"" >> $BASEDIR/rkward/version.h
echo "\".rk.app.version\" <- \"$VERSION\"" > $BASEDIR/rkward/rbackend/rpackages/rkward/R/ver.R
echo "$VERSION" > $BASEDIR/rkward/resource.ver
