#!/bin/bash
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#

## begin: These may need adjusting!
## see http://www.releases.ubuntu.com/ for the up-to-date list
TARGETS="oracular plucky"
AUTHOR="Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>"
## end: These may need adjusting!

cd `dirname $0`/..
BASEDIR=`pwd`
PPATEMPDIR=$BASEDIR/ppatemp
mkdir $PPATEMPDIR

if [ ! -d "$BASEDIR/debian" ]; then
	git clone https://invent.kde.org/tfry/rkward-ppa-support.git debian
	cd debian
else
	cd debian
	git pull
fi
git checkout kf6
cd ..


PPAIDS="rkward-stable rkward-stable-cran rkward-stable-backports-cran"
PPAVERSIONSTRING=".1rkward.stable"
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`

if [ ! -f "${BASEDIR}/rkward-$VERSION.tar.gz" ]; then
  echo "${BASEDIR}/rkward-$VERSION.tar.gz not found. Run makedist.sh, first."
  exit 1
fi
cp ${BASEDIR}/rkward-$VERSION.tar.gz $PPATEMPDIR/rkward_$VERSION.orig.tar.gz

function doSourceUpload {
	TARGET=${1}
	PACKAGEVERSION="${VERSION}-1${PPAVERSIONSTRING}~${TARGET}"

	echo "----------------------"
	echo "Now packaging: ${PACKAGEVERSION}"
	echo "----------------------"

	# unpack
	cd $PPATEMPDIR
	tar -xzf rkward_$VERSION.orig.tar.gz
	PPASOURCEDIR=`pwd`/rkward-$VERSION/
	cp -a $BASEDIR/debian $PPASOURCEDIR

	# prepare changelog
	cd $PPASOURCEDIR/debian
	mv changelog changelog_old
	echo "rkward (${PACKAGEVERSION}) ${TARGET}; urgency=low" > changelog
	echo "  * new upstream development release" >> changelog
	echo -n " -- ${AUTHOR}  " >> changelog
	date -R >> changelog
	echo "" >> changelog
	cat changelog_old >> changelog
	rm changelog_old

	# build source package
	cd $PPASOURCEDIR
	dpkg-buildpackage -S -d -sa -i -I

	# upload
	cd $PPATEMPDIR
	echo "[rkward-devel-scripted]" > dput.cf
	echo "fqdn = ppa.launchpad.net" >> dput.cf
	echo "method = ftp" >> dput.cf
	echo "incoming = ~rkward-devel/${PPAID}/ubuntu/" >> dput.cf
	echo "login = anonymous" >> dput.cf
	echo "allow_unsigned_uploads = 0" >> dput.cf

	dput --config ${PPATEMPDIR}/dput.cf rkward-devel-scripted rkward_${PACKAGEVERSION}_source.changes
	rm -rf rkward_${PACKAGEVERSION}*
	rm -rf $PPASOURCEDIR
}

for TARGET in ${TARGETS}
do
	for PPAID in ${PPAIDS}
	do
		doSourceUpload ${TARGET}
	done
done
