#!/bin/bash

## begin: These may need adjusting!
TARGETS="quantal precise oneiric natty lucid"
AUTHOR="Thomas Friedrichsmeier <tfry@users.sourceforge.net>"
## end: These may need adjusting!

cd `dirname $0`/..
BASEDIR=`pwd`
PPATEMPDIR=$BASEDIR/ppatemp
mkdir $PPATEMPDIR

if [ "${1}" == "--stable" ]; then
	shift
	PPAIDS="rkward-stable rkward-stable-cran"
	PPAVERSIONSTRING=".1rkward.stable"
	# do not re-create tarball
	VERSION=`${BASEDIR}/scripts/getversion.sh ${2}`
else
	PPAIDS="rkward-devel rkward-devel-cran"
	PPAVERSIONSTRING=".0rkward.devel"
	# first create source snapshot
	VERSION=`${BASEDIR}/scripts/getversion.sh ${2}`
	${BASEDIR}/scripts/makedist.sh $VERSION
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
	dpkg-buildpackage -S -sa

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
