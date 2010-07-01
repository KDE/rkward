#!/bin/bash

## begin: These may need adjusting!
OSCREPOS="home:tfry-suse:rkward-devel"
## end: These may need adjusting!

VERSION=${1}
cd `dirname $0`/..
BASEDIR=`pwd`
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
OSCVERSION=`echo -n ${VERSION} | sed -e 's/-/_/g'`
cp ${BASEDIR}/rkward-$VERSION.tar.gz $OSCTEMPDIR/${OSCREPOS}/rkward/rkward-$OSCVERSION.tar.gz
osc add $OSCTEMPDIR/${OSCREPOS}/rkward/rkward-$OSCVERSION.tar.gz

cd $OSCTEMPDIR/${OSCREPOS}/rkward/
sed -i rkward.spec -e "s/Version:.*$/Version:        ${OSCVERSION}/"
osc commit -m "New development snapshot: ${VERSION}"

