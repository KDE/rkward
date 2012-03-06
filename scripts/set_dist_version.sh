#!/bin/bash
#
# Update the version information in the various places around the source
#

VERSION=${1}
cd `dirname $0`/..
BASEDIR=`pwd`

echo "# DO NOT CHANGE THIS FILE MANUALLY!
# It will be overwritten by scripts/set_dist_version.sh
SET(RKVERSION_NUMBER $VERSION)" > $BASEDIR/VERSION.cmake
