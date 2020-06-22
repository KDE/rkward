#! /usr/bin/env python3
# ***************************************************************************
#                          check_translations  -  description
#                             -------------------
#    begin                : Jun 2020
#    copyright            : (C) 2020 by Thomas Friedrichsmeier
#    email                : tfry@users.sourceforge.net
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
# Check translations for obvious problems

import os
import sys
import glob
import polib

SCRIPTDIR = os.path.dirname (os.path.realpath (sys.argv[0]))
EXPORTDIR = os.path.join (SCRIPTDIR, "..", "i18n", "po")

def checkCompleteness(filename):
    po = polib.pofile(filename)
    percentage = po.percent_translated()
    if percentage == 0:
        sys.stderr.write("PURGE: %s has no translated messages.\n" % filename)
        return False
    if percentage < 80:
        sys.stderr.write("WARNING: %s only has %d%% messages translated.\n" % (filename, percentage))
        # Uncomment this line to purge badly incomplete translations:
        #return False
    return True

pofiles = glob.glob(os.path.join(EXPORTDIR, '**', '*.po'), recursive=True)
for pofile in pofiles:
    if not checkCompleteness(pofile):
        os.remove(pofile)
