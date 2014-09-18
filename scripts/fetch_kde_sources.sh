#!/bin/bash
# Script to fetch the source packages belonging to a KDE on Windows installation.
# For creating the source bundle accompanying a Windows binary bundle.
cd $0
BASEDIR=`pwd`

MANIFESTDIR=${BASEDIR}/../RKWard/KDE/manifest/
REPOSITORY="http://winkde.org/pub/kde/ports/win32/repository-4.10/"
SUBDIRS="win32libs kde kdesupport aspell"
SUFFIXES=".tar.bz2 .zip"

FILES=`find ${MANIFESTDIR} -name '*bin.ver' -printf '%f\n' | sed -e "s/bin\.ver/src/"`
FILES="${FILES} `find ${MANIFESTDIR} -name '*bin.ver' -printf '%f\n' | sed -e "s/bin\.ver/lib/"`"

cd ${BASEDIR}/KDE

for FILE in ${FILES}
do
    FOUND="0"

    for DIR in ${SUBDIRS}
    do
	for SUFFIX in ${SUFFIXES}
	do
	    wget -r ${REPOSITORY}/${DIR}/${FILE}${SUFFIX}
	    if [ $? -eq 0 ]; then
		FOUND="1"
		break;
	    fi
	done
	if [ ${FOUND} -eq "1" ]; then
	    break
	fi
    done

    if [ ${FOUND} -eq "0" ]; then
	NOTFOUND="${FILE} ${NOTFOUND}"
    fi
done

if [ "${NOTFOUND}" != "" ]; then
    echo "Did not find these files:"
    for FILE in ${NOTFOUND}
    do
	echo ${FILE}
    done
fi
