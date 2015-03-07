#!/bin/bash

## begin: These may need adjusting!
OSCREPOS="home:tfry-suse:rkward-devel"
## end: These may need adjusting!

cd `dirname $0`/..
BASEDIR=`pwd`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`
if [ ! -f "${BASEDIR}/rkward-$VERSION.tar.gz" ]; then
  echo "${BASEDIR}/rkward-$VERSION.tar.gz not found. Run makedist.sh, first."
  exit 1
fi
# RPM does not accept dashes in the version name...
RPMVERSION=`echo -n ${VERSION} | sed -e 's/-/_/g'`
OSCTEMPDIR=${BASEDIR}/osctemp
rm -rf ${OSCTEMPDIR}
mkdir ${OSCTEMPDIR}

# check out repository state
cd ${OSCTEMPDIR}
osc co ${OSCREPOS}
cd ${OSCREPOS}/rkward
osc remove *.tar.gz

# copy source snapshot
cd ${BASEDIR}
cp ${BASEDIR}/rkward-$VERSION.tar.gz $OSCTEMPDIR/${OSCREPOS}/rkward/rkward-$RPMVERSION.tar.gz
osc add $OSCTEMPDIR/${OSCREPOS}/rkward/rkward-$RPMVERSION.tar.gz

cd $OSCTEMPDIR/${OSCREPOS}/rkward/
sed -i rkward.spec -e "s/Version:.*$/Version:        ${RPMVERSION}/"
osc commit -m "New development snapshot: ${RPMVERSION}"

