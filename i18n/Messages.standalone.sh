#!/bin/sh
BASEDIR=`dirname $0`/../rkward/ # root of translatable sources
export EXTRACTRC=`which extractrc`
export XGETTEXT="xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
                          -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3"
export podir=`dirname $0`

cd ${BASEDIR}/..
/bin/sh Messages.sh
rm rc.cpp

echo "Extracting messages from plugins"
cd ${BASEDIR}
# For testing purposes, extract analysis.pluginmap, only
python ../scripts/update_plugin_messages.py --extract-only --outdir=../i18n/ plugins/analysis.pluginmap
#python ../scripts/extract_plugin_messages.py --outdir=../po/plugins/ --default_po=testing plugins/under_development.pluginmap
echo "Done"
