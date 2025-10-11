#!bin/sh
#!/usr/bin/python3
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
 
# invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources
# the results are stored in a pseudo .cpp file to be picked up by xgettext.
$EXTRACTRC `find rkward -name \*.rc -a \! -name rkward_windows_icon.rc -o -name \*.ui -o -name \*.kcfg`  >> rc.cpp
# merge messages from .R files into rc.cpp
python3 scripts/update_plugin_messages.py rkward/rbackend/rpackages/rkward/R/*.R >> rc.cpp
# call xgettext on all source files in the main app.
$XGETTEXT `find rkward -name \*.cpp -o -name \*.h -name \*.c` rc.cpp -o $podir/rkward.pot

# extract messages from global .rkh pages: >> rkward__pages.pot
python3 scripts/update_plugin_messages.py --extract-only --default_po=pages --outdir=$podir rkward/pages/*.rkh
# extract messages from analysis plugins: >> rkward__analysis.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/analysis.pluginmap
# extract messages from import_export plugins: >> rkward__import_export.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/import_export.pluginmap
# extract messages from data plugins: >> rkward__data.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/data.pluginmap
# extract messages from plot plugins: >> rkward__plots.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/plots.pluginmap
# extract messages from graphics device plugins: >> rkward__graphics_device.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/x11device.pluginmap
# extract messages from IRT plugins: >> rkward__item_response_theory.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/irt.pluginmap
# extract messages from distribution plugins: >> rkward__distributions.pot
python3 scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/distributions.pluginmap
# messages of embedded plugins are extracted implicitly, as part of the above extraction calls: >> rkward__embedded.pot
