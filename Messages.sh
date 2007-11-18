#!/bin/sh
DIR=`pwd`
cd ${BASEDIR}
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' > ${DIR}/infiles.list
cd $DIR
xgettext -C -ci18n -ki18n -ktr2i18n -kI18N_NOOP -kI18N_NOOP2 --files-from=infiles.list -D ${BASEDIR} -o rkward.pot
