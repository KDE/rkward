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

EXTERNAL_REPO=https://github.com/rkward-community/external-plugins.git
SOURCE_PATH=$1
TARGET_PATH=$2

cd `dirname $0`/..
BASEDIR=`pwd`
cd ${BASEDIR}
WORKDIR=${BASEDIR}/import_tmp

# clone and filter external repo
git clone ${EXTERNAL_REPO} import_tmp
cd ${WORKDIR}
git remote rm origin # safety measure
# first split out the target into a single dir. For simplicity, we use ${TARGET_PATH}, here
git filter-branch --prune-empty --tree-filter "
if [ -e ${SOURCE_PATH} ]; then
  mkdir -p ${TARGET_PATH}
  mv ${SOURCE_PATH} ${TARGET_PATH}
fi"
# next filter that single dir
git filter-branch -f --subdirectory-filter ${TARGET_PATH} -- --all
mkdir -p ${TARGET_PATH}
mv * ${TARGET_PATH}
git add .
git commit -m "Import ${SOURCE_PATH} from external_plugins repository"

# merge into main repo
cd ${BASEDIR}
git remote add external_repo_import ${WORKDIR}
git pull --no-rebase external_repo_import master
git remote rm external_repo_import

# Done
rm -rf ${WORKDIR}
git status
