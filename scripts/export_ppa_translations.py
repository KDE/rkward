#! /usr/bin/env python3
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: 2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
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
