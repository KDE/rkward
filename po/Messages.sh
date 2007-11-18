#!/bin/sh
BASEDIR="../rkward/"
DIR=`pwd`

echo "Extracting messages"

cd ${BASEDIR}
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > ${DIR}/infiles.list
cd ${DIR}
xgettext -C -ci18n -ki18n -ktr2i18n -kI18N_NOOP -kI18N_NOOP2 --files-from=infiles.list -D ${BASEDIR} -o rkward.pot
rm infiles.list

echo "Merging translations"

catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  msgmerge -o $cat.new $cat rkward.pot
  mv $cat.new $cat
done