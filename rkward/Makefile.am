# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

bin_PROGRAMS = rkward.bin

rkward_bin_SOURCES = rkwatch.cpp rkward.cpp main.cpp \
	rkglobals.cpp robjectbrowser.cpp rkeditormanager.cpp robjectviewer.cpp \
	khelpdlg.cpp rkconsole.cpp rkward.skel

noinst_HEADERS = rkwatch.h rkward.h debug.h \
	rkglobals.h robjectbrowser.h rkeditormanager.h robjectviewer.h \
	khelpdlg.h rkconsole.h

rkward_bin_LDADD = $(top_builddir)/rkward/windows/libwindows.a \
	$(top_builddir)/rkward/agents/libagents.a $(top_builddir)/rkward/dialogs/libdialogs.a \
	$(top_builddir)/rkward/plugin/libplugin.a $(top_builddir)/rkward/settings/libsettings.a \
	$(top_builddir)/rkward/dataeditor/libdataeditor.a $(top_builddir)/rkward/core/libcore.a \
	$(top_builddir)/rkward/rbackend/librbackend.a $(top_builddir)/rkward/scriptbackends/libscriptbackends.a \
	$(top_builddir)/rkward/misc/libmisc.a -lkhtml -lkmdi $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) \
	$(LIBSOCKET) -L$(R_HOMEDIR)/lib -lR -lRlapack -lkatepartinterfaces

SUBDIRS = plugins dataeditor settings plugin agents rbackend scriptbackends \
	misc core dialogs icons windows

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/rkward.desktop $(kde_appsdir)/Applications/rkward.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-rkward.png $(kde_icondir)/locolor/32x32/apps/rkward.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-rkward.png $(kde_icondir)/locolor/16x16/apps/rkward.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/rkward.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/rkward.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/rkward.png

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

METASOURCES = AUTO

# the library search path. 
rkward_bin_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/rkward
rc_DATA = rkwardui.rc rkconsolepart.rc

bin_SCRIPTS = rkward

# all the perl script does is fill in the placeholder for R_HOMEDIR and friends
rkward:
	perl ./makerkwardscript.pl rkward.sh.template "$(R_HOMEDIR)" > rkward; \
	chmod 744 rkward

CLEANFILES = rkward

EXTRA_DIST = makerkwardscript.pl rkward.sh.template

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/rkward.pot; \
	fi

phpfilesdir = $(kde_datadir)/rkward/phpfiles

katesyntaxdir = $(kde_datadir)/katepart/syntax/
katesyntax_DATA =  r.xml rkward.xml
