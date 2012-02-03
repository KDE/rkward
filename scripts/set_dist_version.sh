#!/bin/bash
#
# Update the version information in the various places around the source
#

VERSION=${1}
cd `dirname $0`/..
BASEDIR=`pwd`

echo "SET(RKVERSION_NUMBER $VERSION)" > $BASEDIR/VERSION.cmake
