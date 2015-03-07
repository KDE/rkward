#!/bin/bash
cd `dirname $0`/..
BASEDIR=`pwd`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`
DEBTEMPDIR=${BASEDIR}/debiantemp
rm -rf ${DEBTEMPDIR}
mkdir ${DEBTEMPDIR}

if [ -z "`head -n 1 ${BASEDIR}/debian/changelog | grep "rkward (${VERSION}-"`" ]; then
  echo "Version mismatch! Adjust ${BASEDIR}/debian/changelog, first."
  exit 1
fi

# check for source snapshot
if [ ! -f ${BASEDIR}/rkward-$VERSION.tar.gz ]; then
  echo "${BASEDIR}/rkward-$VERSION.tar.gz not found. Run makedist.sh, first."
  exit 1
fi

cp ${BASEDIR}/rkward-$VERSION.tar.gz $DEBTEMPDIR/rkward_$VERSION.orig.tar.gz
cd $DEBTEMPDIR
tar -xzf rkward_$VERSION.orig.tar.gz
cd rkward-$VERSION
cp -a ${BASEDIR}/debian .
dpkg-buildpackage -k0x1858CBB6 -rfakeroot

cd $DEBTEMPDIR
dpkg-scansources . | bzip2 > Sources.bz2
LINTIAN_PROFILE=debian lintian rkward_$VERSION-*.changes
