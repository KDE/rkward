#!/bin/bash
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Update the version information in the various places around the source
#

SPECIFIEDVERSION=${1}
cd `dirname $0`/..
BASEDIR=`pwd`

if [ -z ${SPECIFIEDVERSION} ]; then
	awk '{ if ($0 ~ /^SET/) { sub(/SET\(RKVERSION_NUMBER /, ""); sub(/\)/, ""); printf "%s", $0; }; next }' VERSION.cmake
elif [ ${SPECIFIEDVERSION} = "SVN" ]; then
	VERSION=`${BASEDIR}/scripts/getversion.sh`
	cd ${BASEDIR}
	REVISION=`svn info | grep "Revision:" | sed -e "s/Revision: //"`
	echo -n "${VERSION}-SVN${REVISION}"
else
	echo -n ${SPECIFIEDVERSION}
fi
