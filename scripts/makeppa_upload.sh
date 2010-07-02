#!/bin/bash

## begin: These may need adjusting!
TARGETS="maverick lucid karmic jaunty"
PPAVERSIONSTRING="experimental1ppa1"
PPAID="rkward-devel"
AUTHOR="Thomas Friedrichsmeier <tfry@users.sourceforge.net>"
## end: These may need adjusting!

cd `dirname $0`/..
BASEDIR=`pwd`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`
PPATEMPDIR=$BASEDIR/ppatemp
mkdir $PPATEMPDIR

# first create source snapshot
${BASEDIR}/scripts/makedist.sh $VERSION
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
	dpkg-buildpackage -S

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
	doSourceUpload ${TARGET}
done
