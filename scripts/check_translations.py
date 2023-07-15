#! /usr/bin/env python3
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: 2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Check translations for obvious problems

import os
import sys
import glob
import polib

SCRIPTDIR = os.path.dirname(os.path.realpath(sys.argv[0]))
BASEDIR = os.path.join(SCRIPTDIR, "..")

strictmode = False
paths = []
for arg in sys.argv[1:]:
    if arg == "--strict":
        strictmode = True
    else:
        paths.append(arg)
if (len(paths) < 1):
    paths = [os.path.join(BASEDIR, "po")]

def checkCompleteness(filename):
    po = polib.pofile(filename)
    # do not use po.percent_translated(), because it returns an integer
    # percentage, so we cannot distiguish between 0 translated messages
    # and e.g. 0.9% translated
    stringcount = 0
    transcount = 0
    for entry in po:
        if entry.obsolete:
            continue
        stringcount += 1
        if entry.translated():
            transcount += 1
    if transcount == 0:
        sys.stderr.write("PURGE: %s has no translated messages.\n" % filename)
        return False
    percentage = (transcount / stringcount) * 100.0
    if percentage < 70:
        sys.stderr.write("WARNING: %s only has %.2f%% messages translated.\n" % (filename, percentage))
        if (strictmode):
            sys.stderr.write("PURGE: %s\n" % filename)
            return False
    return True

for path in paths:
    if os.path.isdir(path):
        pofiles = glob.glob(os.path.join(path, '**', '*.po'), recursive=True)
        for pofile in pofiles:
            if not checkCompleteness(pofile):
                os.remove(pofile)
