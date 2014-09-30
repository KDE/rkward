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

# create source snapshot
if [ ! -f ${BASEDIR}/rkward-$VERSION.tar.gz ]; then
  cd ${BASEDIR}
  ${BASEDIR}/scripts/makedist.sh $VERSION
fi

cp ${BASEDIR}/rkward-$VERSION.tar.gz $DEBTEMPDIR/rkward_$VERSION.orig.tar.gz
cd $DEBTEMPDIR
tar -xzf rkward_$VERSION.orig.tar.gz
cd rkward-$VERSION
cp -a ${BASEDIR}/debian .
dpkg-buildpackage -rfakeroot

cd $DEBTEMPDIR
LINTIAN_PROFILE=debian lintian rkward_$VERSION-*.changes
