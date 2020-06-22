#! /usr/bin/env python3
# ***************************************************************************
#                          import_translations  -  description
#                             -------------------
#    begin                : Jan 2015
#    copyright            : (C) 2015 by Thomas Friedrichsmeier
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
# Somewhat experimental script to import translations from KDE l10n infrastructure.
# Imports specified - or all - languages to tmp/export .
# Imported po-files are renamed according to rkward's naming scheme.
# Modelled - to some degree - after l10n.rb from releaseme.

import sys
import subprocess
import os
import re
import shutil

SVNROOT = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"
RKWARDSVNPATH = "messages/rkward"
SCRIPTDIR = os.path.dirname (os.path.realpath (sys.argv[0]))
TMPDIR = os.path.join (SCRIPTDIR, "tmp")
EXPORTDIR = os.path.join (SCRIPTDIR, "..", "i18n")
IGNOREDPONAMES = {'org.kde.rkward.appdata.po', 'rkward._desktop_.po', 'rkward_xml_mimetypes.po'}

if not os.path.exists (TMPDIR):
    os.makedirs (TMPDIR)
if not os.path.exists (EXPORTDIR):
    os.makedirs (os.path.join (EXPORTDIR))

if (len (sys.argv) > 1):
    LANGUAGES = sys.argv[1:]
else:
    LANGUAGES = subprocess.check_output (["svn", "cat", SVNROOT + "subdirs"]).decode ('utf-8').split ()
    LANGUAGES.remove ('x-test')
LANGUAGES = LANGUAGES
print ("Languages: " + ", ".join (LANGUAGES))

for lang in LANGUAGES:
    os.chdir (TMPDIR)
    langdir = os.path.join (TMPDIR, lang)
    if not os.path.exists (langdir):
        subprocess.call (["svn", "co", SVNROOT + lang + "/" + RKWARDSVNPATH, lang])
        if not os.path.exists (langdir):
            continue
    else:
        os.chdir (langdir)
        subprocess.call (["svn", "up"])
        os.chdir (TMPDIR)
    pofiles = [fn for fn in os.listdir (langdir) if fn.endswith ('.po') and fn not in IGNOREDPONAMES]
    if (len (pofiles) < 1):
        continue
    for pofile in pofiles:
        outfile = os.path.join (EXPORTDIR, re.sub ("po$", lang + ".po", pofile))
        infile = os.path.join (langdir, pofile)

        # copy to destination
        print ("writing " + outfile)
        shutil.copyfile (infile, outfile)
