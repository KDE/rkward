#!/bin/bash
#
# Update the version information in the various places around the source
#

SPECIFIEDVERSION=${1}
cd `dirname $0`/..
BASEDIR=`pwd`

if [ -z ${SPECIFIEDVERSION} ]; then
	awk '{ printf "%s", $0; next }' rkward/resource.ver
elif [ ${SPECIFIEDVERSION} = "SVN" ]; then
	VERSION=`${BASEDIR}/scripts/getversion.sh`
	cd ${BASEDIR}
	REVISION=`svn info | grep "Revision:" | sed -e "s/Revision: //"`
	echo -n "${VERSION}-SVN${REVISION}"
else
	echo -n ${SPECIFIEDVERSION}
fi
