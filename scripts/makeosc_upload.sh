#!/bin/bash

## begin: These may need adjusting!
OSCREPOS="home:tfry-suse:rkward-devel"
## end: These may need adjusting!

cd `dirname $0`/..
BASEDIR=`pwd`
VERSION=`${BASEDIR}/scripts/getversion.sh ${1}`
# RPM does not accept dashes in the version name...
VERSION=`echo -n ${VERSION} | sed -e 's/-/_/g'`
OSCTEMPDIR=${BASEDIR}/osctemp
rm -rf ${OSCTEMPDIR}
mkdir ${OSCTEMPDIR}

# check out repository state
cd ${OSCTEMPDIR}
osc co ${OSCREPOS}
cd ${OSCREPOS}/rkward
osc remove *.tar.gz

# create source snapshot
cd ${BASEDIR}
${BASEDIR}/scripts/makedist.sh $VERSION
cp ${BASEDIR}/rkward-$VERSION.tar.gz $OSCTEMPDIR/${OSCREPOS}/rkward/rkward-$VERSION.tar.gz
osc add $OSCTEMPDIR/${OSCREPOS}/rkward/rkward-$VERSION.tar.gz

cd $OSCTEMPDIR/${OSCREPOS}/rkward/
sed -i rkward.spec -e "s/Version:.*$/Version:        ${VERSION}/"
osc commit -m "New development snapshot: ${VERSION}"

