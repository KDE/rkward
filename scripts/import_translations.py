#! /usr/bin/python
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
# Imported po-files are renamed according to rkward's naming scheme, and all
# po-file comments are stripped / replaced with a message to discourage accidental
# editing.
# Modelled - to some degree - after l10n.rb from releaseme.

import sys
import subprocess
import os
import codecs
import re

SVNROOT = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"
RKWARDSVNPATH = "messages/rkward"
SCRIPTDIR = os.path.dirname (os.path.realpath (sys.argv[0]))
TMPDIR = os.path.join (SCRIPTDIR, "tmp")
EXPORTDIR = os.path.join (SCRIPTDIR, "..", "i18n", "po")
IGNOREDPONAMES = {'org.kde.rkward.appdata.po', 'rkward._desktop_.po', 'rkward_xml_mimetypes.po'}
if not os.path.exists (TMPDIR):
    os.makedirs (TMPDIR)
if not os.path.exists (EXPORTDIR):
    os.makedirs (os.path.join (EXPORTDIR))

if (len (sys.argv) > 1):
    LANGUAGES = sys.argv[1:]
else:
    LANGUAGES = subprocess.check_output (["svn", "cat", SVNROOT + "subdirs"]).split ()
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

        # copy to destination, and strip unneeded comments
        print ("writing " + outfile)
        pf = codecs.open (infile, 'r', 'utf-8')
        of = codecs.open (outfile, 'w', 'utf-8')
        prev_was_comment = False
        for line in pf:
            if (line.startswith ("#")):
                if (line.startswith ("#:")):
                    if not prev_was_comment:
                        of.write ("#: translation_export.do_not_modify_here:0\n")
                        prev_was_comment = True
                elif (line.startswith ("#,")):
                    of.write (line)
                else:
                    continue
            else:
                of.write (line)
                prev_was_comment = False
        pf.close ()
        of.close ()
