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
import shutil

SVNROOT = "svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/"
RKWARDSVNPATH = "messages/rkward"
RKWARDSVNDOCSPATH = "docs/rkward"
SCRIPTDIR = os.path.dirname (os.path.realpath (sys.argv[0]))
TMPDIR = os.path.join (SCRIPTDIR, "tmp")
I18NDIR = os.path.join (SCRIPTDIR, "..", "i18n")
EXPORTDIR = os.path.join (I18NDIR, "po")
PODIR = os.path.join (SCRIPTDIR, "..", "po")
IGNOREDPONAMES = {'org.kde.rkward.appdata.po', 'rkward._desktop_.po', 'rkward_xml_mimetypes.po'}
SVNCMD = shutil.which("svn")  # could be svn.BAT on Windows/craft, and that won't be found by subprocess.call

if not os.path.exists (TMPDIR):
    os.makedirs (TMPDIR)
if os.path.exists (EXPORTDIR):
    shutil.rmtree (os.path.join (EXPORTDIR))

if (len (sys.argv) > 1):
    LANGUAGES = sys.argv[1:]
else:
    LANGUAGES = subprocess.check_output ([SVNCMD, "cat", SVNROOT + "subdirs"]).decode ('utf-8').split ()
    LANGUAGES.remove ('x-test')
print ("Languages: " + ", ".join (LANGUAGES))

for lang in LANGUAGES:
    os.chdir (TMPDIR)
    messagesdir = os.path.join (TMPDIR, "messages-" + lang)
    if not os.path.exists (messagesdir):
        subprocess.call ([SVNCMD, "co", SVNROOT + lang + "/" + RKWARDSVNPATH, "messages-" + lang])
        if not os.path.exists (messagesdir):
            continue
    else:
        os.chdir (messagesdir)
        subprocess.call ([SVNCMD, "up"])
        os.chdir (TMPDIR)
    pofiles = [fn for fn in os.listdir (messagesdir) if fn.endswith ('.po') and fn not in IGNOREDPONAMES]
    if (len (pofiles) < 1):
        continue
    langpodir = os.path.join (PODIR, lang)
    exportlangpodir = os.path.join (EXPORTDIR, lang)
    for pofile in pofiles:
        is_main = pofile == "rkward.po"
        if is_main:
            outdir = langpodir
        else:
            outdir = exportlangpodir
        infile = os.path.join (messagesdir, pofile)
        outfile = os.path.join (outdir, pofile)
        if not os.path.exists (outdir):
            os.makedirs (outdir)
            if not is_main:
                shutil.copyfile (os.path.join (I18NDIR, "compile_lang.cmake"), os.path.join (outdir, "CMakeLists.txt"))

        # copy to destination
        print ("writing " + outfile)
        shutil.copyfile (infile, outfile)

for lang in LANGUAGES:
    os.chdir (TMPDIR)
    docsdir = os.path.join (TMPDIR, "docs-" + lang)
    if not os.path.exists (docsdir):
        subprocess.call ([SVNCMD, "co", SVNROOT + lang + "/" + RKWARDSVNDOCSPATH, "docs-" + lang])
        if not os.path.exists (docsdir):
            continue
    else:
        os.chdir (docsdir)
        subprocess.call ([SVNCMD, "up"])
        os.chdir (TMPDIR)
    docdirs = [fn for fn in os.listdir (docsdir) if fn[0] != '.' and os.path.isdir (os.path.join (docsdir, fn))]
    if (len (docdirs) < 1):
        continue
    langdocdir = os.path.join (PODIR, lang, "docs")
    for docdir in docdirs:
        indir = os.path.join (docsdir, docdir)
        outdir = os.path.join (langdocdir, docdir)

        # copy to destination
        print ("copying " + outdir)
        if os.path.exists (outdir):
            shutil.rmtree (outdir)
        shutil.copytree (indir, outdir)
