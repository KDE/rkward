#!/bin/bash
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Update the version information in the various places around the source
#

VERSION=${1}
if [ "`echo -n ${VERSION} | grep "\\+"`" != "" ]; then
  TARGET_VERSION=`echo -n ${VERSION} | sed -e 's/[^+]*+\([^+]*\).*/\1/g'`
  PRIOR_VERSION=`echo -n ${VERSION} | sed -e 's/\([^+]*\).*/\1/g'`
else
  TARGET_VERSION=${VERSION}
  PRIOR_VERSION=${VERSION}
fi

echo "Full version name: ${VERSION}"
echo "Target version name: ${TARGET_VERSION}"
echo "Prior version name: ${PRIOR_VERSION}"

cd `dirname $0`/..
BASEDIR=`pwd`

echo "# DO NOT CHANGE THIS FILE MANUALLY!
# It will be overwritten by scripts/set_dist_version.sh
SET(RKVERSION_NUMBER $VERSION)" > $BASEDIR/VERSION.cmake

cd ${BASEDIR}/rkward/plugins/
for pluginmap in *.pluginmap pluginmap_meta.inc; do
   sed -i -e "s/\(\s\)version=\"[^\"]*\"/\1version=\"${TARGET_VERSION}\"/" \
          -e "s/rkward_min_version=\"[^\"]*\"/rkward_min_version=\"${PRIOR_VERSION}\"/" \
          -e "s/rkward_max_version=\"[^\"]*\"/rkward_max_version=\"${TARGET_VERSION}y\"/" $pluginmap
done

sed -i -e "s/^Version:.*$/Version: ${TARGET_VERSION}/" \
       -e "s/^Date:.*$/Date: `date +%Y-%m-%d`/" ${BASEDIR}/rkward/rbackend/rpackages/rkward/DESCRIPTION
