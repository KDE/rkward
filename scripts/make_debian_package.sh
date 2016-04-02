#!/bin/bash
cd `dirname $0`/..
BASEDIR=`pwd`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`
DEBSUFFIX=${2}
DEBDIR=${BASEDIR}/debian-official
DEBTEMPDIR=${BASEDIR}/debiantemp
rm -rf ${DEBTEMPDIR}
mkdir ${DEBTEMPDIR}

if [ -z "`head -n 1 ${DEBDIR}/changelog | grep "rkward (${VERSION}${DEBSUFFIX}-"`" ]; then
  echo "Version mismatch! Adjust ${DEBDIR}/changelog, first."
  exit 1
fi

# check for source snapshot
if [ ! -f ${BASEDIR}/rkward-$VERSION.tar.gz ]; then
  echo "${BASEDIR}/rkward-$VERSION.tar.gz not found. Run makedist.sh, first."
  exit 1
fi

cp ${BASEDIR}/rkward-$VERSION.tar.gz $DEBTEMPDIR/rkward_$VERSION${DEBSUFFIX}.orig.tar.gz
cd $DEBTEMPDIR
tar -xzf rkward_$VERSION${DEBSUFFIX}.orig.tar.gz
cd rkward-$VERSION
cp -a ${DEBDIR} debian
dpkg-buildpackage -k0x1858CBB6 -rfakeroot --force-sign

cd $DEBTEMPDIR
dpkg-scansources . | bzip2 > Sources.bz2
LINTIAN_PROFILE=debian lintian rkward_$VERSION${DEBSUFFIX}-*.changes
