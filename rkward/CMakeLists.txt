ADD_SUBDIRECTORY( dataeditor )
ADD_SUBDIRECTORY( settings )
ADD_SUBDIRECTORY( plugin )
ADD_SUBDIRECTORY( agents )
ADD_SUBDIRECTORY( rbackend )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( scriptbackends )
ADD_SUBDIRECTORY( misc )
ADD_SUBDIRECTORY( core )
ADD_SUBDIRECTORY( icons )
ADD_SUBDIRECTORY( windows )
ADD_SUBDIRECTORY( syntax )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

SET(RKWard_Sources
	rkward.cpp
	main.cpp
	rkglobals.cpp
	robjectbrowser.cpp
	robjectviewer.cpp
	rkconsole.cpp
	rkwardapplication.cpp
)

########### next target ###############

# auto-gen sources
QT4_AUTOMOC(${RKWard_Sources})

# somehow the needed libdirs for R do not get passed down automatically
GET_DIRECTORY_PROPERTY(R_SHAREDLIBDIR DIRECTORY rbackend LINK_DIRECTORIES)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

KDE4_ADD_EXECUTABLE(rkward.bin ${RKWard_Sources})

TARGET_LINK_LIBRARIES(rkward.bin ${KDE4_KDECORE_LIBS} windows agents dialogs plugin settings dataeditor core scriptbackends rbackend misc ${KDE4_KHTML_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS})

# NOTE: KDE4_KFILE_LIBS still links against Qt3Support

# wrapper script
GET_DIRECTORY_PROPERTY(R_HOME DIRECTORY rbackend DEFINITION R_HOME)
# TODO: somehow this is built unconditionally, but nevermind
ADD_CUSTOM_TARGET(rkwardwrapper
	COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/makerkwardscript.pl ${CMAKE_CURRENT_SOURCE_DIR}/rkward.sh.template "${R_HOME}" > ${CMAKE_CURRENT_BINARY_DIR}/rkward
	DEPENDS makerkwardscript.pl rkward.sh.template
)
# will this work?
#CONFIGURE_FILE(
#	rkward.sh.template
#	${CMAKE_CURRENT_BINARY_DIR}/rkward
#	@ONLY)
ADD_DEPENDENCIES(rkward.bin rkwardwrapper)

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${DATA_INSTALL_DIR}/rkward
	PATTERN .svn EXCLUDE)

INSTALL(FILES   rkwardui.rc rkconsolepart.rc DESTINATION ${DATA_INSTALL_DIR}/rkward )
INSTALL(TARGETS rkward.bin DESTINATION ${BIN_INSTALL_DIR})
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rkward DESTINATION ${BIN_INSTALL_DIR})

# TODO:
#INSTALL(FILES rkward.desktop DESTINATION ${XDG_APPS_DIR})
# icons?!

# TODO:
#install-data-local:
#	$(mkinstalldirs) $(kde_appsdir)/Applications/
#	$(INSTALL_DATA) $(srcdir)/rkward.desktop $(kde_appsdir)/Applications/rkward.desktop
#	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
#	$(INSTALL_DATA) $(srcdir)/lo32-app-rkward.png $(kde_icondir)/locolor/32x32/apps/rkward.png
#	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
#	$(INSTALL_DATA) $(srcdir)/lo16-app-rkward.png $(kde_icondir)/locolor/16x16/apps/rkward.png
#
#uninstall-local:
#	-rm -f $(kde_appsdir)/Applications/rkward.desktop
#	-rm -f $(kde_icondir)/locolor/32x32/apps/rkward.png
#	-rm -f $(kde_icondir)/locolor/16x16/apps/rkward.png
#
