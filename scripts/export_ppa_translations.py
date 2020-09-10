#! /usr/bin/env python3
# ***************************************************************************
#                          export_ppa_translations  -  description
#                             -------------------
#    begin                : Sep 09 2020
#    copyright            : (C) 2020 by Thomas Friedrichsmeier
#    email                : thomas.friedrichsmeier@kdemail.net
# ***************************************************************************
#
# ***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************
#
# export translations for use in Launchpad PPAs

import sys
import subprocess
import os
import shutil

EXPORTREPO = "git@invent.kde.org:tfry/rkward-po-export.git"
SCRIPTDIR = os.path.dirname(os.path.realpath(sys.argv[0]))
EXPORTDIR = os.path.join(SCRIPTDIR, "poexport")
BASEDIR = os.path.join(SCRIPTDIR, "..")
COPYDIRS = ["po", os.path.join("i18n", "po")]

os.chdir (SCRIPTDIR)
if not os.path.exists (EXPORTDIR):
    subprocess.call (["git", "clone", EXPORTREPO, EXPORTDIR])
else:
    os.chdir (EXPORTDIR)
    subprocess.call (["git", "pull", "--rebase"])

for dir in COPYDIRS:
    dest = os.path.join(EXPORTDIR, dir)
    shutil.rmtree(dest, ignore_errors=True)
    shutil.copytree(os.path.join(BASEDIR, dir), dest)
    subprocess.call([sys.executable, os.path.join(SCRIPTDIR, "check_translations.py"), dest]) #, "--strict"

subprocess.call (["git", "add", "--all"])
subprocess.call (["git", "commit", "-m", "\"Translation update\""])
subprocess.call (["git", "push"])
