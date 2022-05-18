#!/bin/bash
#
# Import a plugin from our external plugins repository, keeping all commit history.
# Actually, imports single specified file or path from that repo. Just pick the
# generator script, and you should be fine (no need to preserve history on generated files).
#
# Note:
# Based on http://gbayer.com/development/moving-files-from-one-git-repository-to-another-preserving-history/
# with some additional tweaking
#
# Usage:
# import_external_plugin.sh source_path target_dir
# Example:
# import_external_plugin.sh rk.power/inst/rkward/rkwarddev_power_plugin_script.R rkward/plugins/rkwarddev_scripts

EXTERNAL_REPO=https://github.com/rkward-community/rk.gitInstall.git
SOURCE_PATH=$1
TARGET_PATH=$2

cd `dirname $0`
SCRIPTDIR=`pwd`
cd ${SCRIPTDIR}/..
BASEDIR=`pwd`
cd ${BASEDIR}
WORKDIR=${BASEDIR}/import_tmp

# Make sure to work on an up-to-date (pushable, without fast-forwards) clone
git pull --rebase

# clone and filter external repo
git clone ${EXTERNAL_REPO} import_tmp
cd ${WORKDIR}
#git remote rm origin # safety measure
git-filter-repo --path ${SOURCE_PATH} --path-rename 'inst/rkward/':${TARGET_PATH}
git-filter-repo --mailmap ${SCRIPTDIR}/committer_map.txt

# merge into main repo
cd ${BASEDIR}
git remote add external_repo_import ${WORKDIR}
git pull --no-rebase external_repo_import master --allow-unrelated-histories
git remote rm external_repo_import

# Done
rm -rf ${WORKDIR}
git status
