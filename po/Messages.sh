#!/bin/sh
BASEDIR="../rkward/"	# root of translatable sources
PROJECT="rkward"	# project name
WDIR=`pwd`		# working dir


echo "Preparing rc files"
cd ${BASEDIR}
# we use simple sorting to make sure the lines don't jump around too much from system to system
find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > ${WDIR}/rcfiles.list
xargs --arg-file=${WDIR}/rcfiles.list extractrc > ${WDIR}/extractedrc.cpp
cd ${WDIR}
echo "Done preparing rc files"


echo "Extracting messages"
cd ${BASEDIR}
# see above on sorting
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > ${WDIR}/infiles.list
echo "extractedrc.cpp" >> ${WDIR}/infiles.list
cd ${WDIR}
xgettext -C -ci18n -ki18n -kki18n -ktr2i18n -kI18N_NOOP -kI18N_NOOP2 --files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot
echo "Done extracting messages"


echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  echo $cat
  msgmerge -o $cat.new $cat ${PROJECT}.pot
  mv $cat.new $cat
done
echo "Done merging translations"


echo "Cleaning up"
cd ${WDIR}
rm rcfiles.list
rm infiles.list
rm extractedrc.cpp
echo "Done"
