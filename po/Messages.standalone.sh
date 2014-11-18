#!/bin/sh
BASEDIR="../rkward/"	# root of translatable sources
PROJECT="rkward"	# project name
BUGADDR="http://p.sf.net/rkward/bugs"	# MSGID-Bugs
WDIR=`pwd`		# working dir

check_error()
{
	if [ "$?" -ne "0" ]; then
		echo "ERROR: ${1}"
		exit 1
	fi
}

echo "Preparing rc files"
cd ${BASEDIR}
# we use simple sorting to make sure the lines don't jump around too much from system to system
find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > ${WDIR}/rcfiles.list
xargs --arg-file=${WDIR}/rcfiles.list extractrc > ${WDIR}/rc.cpp
check_error "Failed to extract messages from rc files. Do you have extractrc installed?"
# additional string for KAboutData
echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> ${WDIR}/rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> ${WDIR}/rc.cpp
cd ${WDIR}
echo "Done preparing rc files"


echo "Extracting messages"
cd ${BASEDIR}
# see above on sorting
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > ${WDIR}/infiles.list
echo "rc.cpp" >> ${WDIR}/infiles.list
cd ${WDIR}
xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
	-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
	--msgid-bugs-address="${BUGADDR}" \
	--files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot
check_error "Failed to extract messages from source files. Do you have xgettext installed?"
echo "Done extracting messages"


# Note: .po-file are now maintained (and merged) on launchpad
#
# echo "Merging translations"
# catalogs=`find . -name '*.po'`
# for cat in $catalogs; do
#   echo $cat
#   msgmerge -o $cat.new $cat ${PROJECT}.pot
#   check_error "Failed to merge messages. Do you have msgmerge installed?"
#   mv $cat.new $cat
# done
# echo "Done merging translations"


echo "Cleaning up"
cd ${WDIR}
rm rcfiles.list
rm infiles.list
rm rc.cpp
echo "Done"

echo "Extracting messages from plugins"
cd ${BASEDIR}
# For testing purposes, extract analysis.pluginmap, only
python ../scripts/extract_plugin_messages.py --outdir=../po/plugins/ plugins/analysis.pluginmap
python ../scripts/extract_plugin_messages.py --outdir=../po/plugins/ --default_po=testing plugins/under_development.pluginmap
echo "Done"
