#!bin/sh
 
# invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources
# the results are stored in a pseudo .cpp file to be picked up by xgettext.
$EXTRACTRC `find rkward -name \*.rc -a \! -name rkward_windows_icon.rc -o -name \*.ui -o -name \*.kcfg`  >> rc.cpp
#
# call xgettext on all source files. If your sources have other filename
# extensions besides .cc, .cpp, and .h, just add them in the find call.
$XGETTEXT `find rkward -name \*.cpp -o -name \*.h -name \*.c` rc.cpp -o $podir/rkward.pot

# extract messages from global .rkh pages: >> rkward__pages.pot
python scripts/update_plugin_messages.py --extract-only --default_po=pages --outdir=$podir rkward/pages/*.rkh
# extract messages from analysis plugins: >> rkward__analysis.pot
python scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/analysis.pluginmap
# extract messages from import_export plugins: >> rkward__import_export.pot
python scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/import_export.pluginmap
# extract messages from data plugins: >> rkward__data.pot
python scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/data.pluginmap
# extract messages from data plugins: >> rkward__plots.pot
python scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/plots.pluginmap
# extract messages from data plugins: >> rkward__item_response_theory.pot
python scripts/update_plugin_messages.py --extract-only --outdir=$podir rkward/plugins/irt.pluginmap
# messages of embedded plugins are extracted implicitly, as part of the above extraction calls: >> rkward__embedded.pot
