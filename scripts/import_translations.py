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

SVNROOT = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kde4/"
RKWARDSVNPATH = "messages/playground-edu/"
SCRIPTDIR = os.path.dirname (os.path.realpath (sys.argv[0]))
TMPDIR = os.path.join (SCRIPTDIR, "tmp")
EXPORTDIR = os.path.join (SCRIPTDIR, "tmp", "export")
if not os.path.exists (TMPDIR):
    os.makedirs (TMPDIR)
if not os.path.exists (EXPORTDIR):
    os.makedirs (os.path.join (EXPORTDIR, "plugins"))

if (len (sys.argv) > 1):
    LANGUAGES = sys.argv[1:]
else:
    LANGUAGES = subprocess.check_output (["svn", "cat", SVNROOT + "subdirs"]).split ()
LANGUAGES = LANGUAGES
print ("Languages: " + ", ".join (LANGUAGES))

PONAMES = []
messagessh = codecs.open (os.path.join (SCRIPTDIR, "..", "Messages.sh"), 'r', 'utf-8')
for line in messagessh:
    match = re.search ("(rkward[^\s]*)\.pot", line)
    if (match != None):
        PONAMES.append (match.group (1) + ".po")
print ("POs: " + ", ".join (PONAMES))
PONAMES = set (PONAMES)
messagessh.close ()

for lang in LANGUAGES:
    os.chdir (TMPDIR)
    try:
        pofiles = subprocess.check_output (["svn", "list", SVNROOT + lang + "/" + RKWARDSVNPATH]).split ('\n')
    except:
        continue
    pofiles = list (set (pofiles) & PONAMES)
    if (len (pofiles) < 1):
        continue
    langdir = os.path.join (TMPDIR, lang)
    if not os.path.exists (langdir):
        subprocess.call (["svn", "co", SVNROOT + lang + "/" + RKWARDSVNPATH, lang, "--depth", "empty"])
    os.chdir (langdir)
    subprocess.call (["svn", "up"] + pofiles)
    os.chdir (TMPDIR)
    for pofile in pofiles:
        if (pofile == "rkward.po"):
            outfile = os.path.join (EXPORTDIR, "rkward." + lang + ".po")
        else:
            outfile = os.path.join (EXPORTDIR, "plugins", re.sub ("po$", lang + ".po", pofile))
        print ("writing " + outfile)
        pf = codecs.open (os.path.join (langdir, pofile), 'r', 'utf-8')
        of = codecs.open (outfile, 'w', 'utf-8')
        prev_was_comment = False
        for line in pf:
            if (line.startswith ("#")):
                if (line.startswith ("#:")):
                    if not prev_was_comment:
                        of.write ("#: translation_export.do_not_modify_here:0\n")
                        prev_was_comment = True
                else:
                    continue
            else:
                of.write (line)
                prev_was_comment = False
        pf.close ()
        of.close ()
