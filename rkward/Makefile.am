####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = rkward

## INCLUDES were found outside kdevelop specific part

rkward_SOURCES = rkradio.cpp rkvarslot.cpp rkvarselector.cpp rkpluginwidget.cpp rkplugin.cpp rkmenu.cpp rsettingsui.ui rsettings.cpp nameselectcell.cpp rtableitem.cpp typeselectcell.cpp rkwatch.cpp rkwatch_ui.ui rinterface.cpp rkdrag.cpp twintablemember.cpp twintable.cpp view.ui rkwarddoc.cpp rkward.cpp main.cpp 
rkward_LDADD   =  $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = plugins 

EXTRA_DIST = main.cpp rkward.cpp rkward.h rkwarddoc.cpp rkwarddoc.h rkwardui.rc rkward.desktop lo32-app-rkward.png lo16-app-rkward.png view.ui twintable.cpp twintable.h twintablemember.cpp twintablemember.h rkdrag.cpp rkdrag.h rinterface.cpp rinterface.h rkwatch_ui.ui rkwatch.cpp rkwatch.h typeselectcell.cpp typeselectcell.h rtableitem.cpp rtableitem.h nameselectcell.cpp nameselectcell.h rsettings.cpp rsettings.h rsettingsui.ui rkmenu.cpp rkmenu.h rkplugin.cpp rkplugin.h rkpluginwidget.cpp rkpluginwidget.h rkvarselector.cpp rkvarselector.h rkvarslot.cpp rkvarslot.h rkradio.cpp rkradio.h 

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

####### kdevelop will overwrite this part!!! (end)############
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

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
rkward_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/rkward
rc_DATA = rkwardui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/rkward.pot; \
	fi

