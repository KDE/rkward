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

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${X11_X11_INCLUDE_PATH} )

SET(RKWard_Sources
	rkward.cpp
	main.cpp
	rkglobals.cpp
	robjectbrowser.cpp
	robjectviewer.cpp
	rkconsole.cpp
	rkwardapplication.cpp
)

# auto-gen sources
#QT4_AUTOMOC(${RKWard_Sources})	# not needed here, as KDE4_ADD_EXECUTABLE mocs the sources in this dir.

# somehow the needed libdirs for R do not get passed down automatically
GET_DIRECTORY_PROPERTY(R_SHAREDLIBDIR DIRECTORY rbackend LINK_DIRECTORIES)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

KDE4_ADD_EXECUTABLE(rkward.bin ${RKWard_Sources})
# wrapper script
GET_DIRECTORY_PROPERTY(R_HOME DIRECTORY rbackend DEFINITION R_HOME)
CONFIGURE_FILE(
	rkward.sh.template
	${CMAKE_CURRENT_BINARY_DIR}/rkward
	@ONLY)
ADD_DEPENDENCIES(rkward.bin ${CMAKE_CURRENT_BINARY_DIR}/rkward)

TARGET_LINK_LIBRARIES(rkward.bin ${KDE4_KDECORE_LIBS} windows agents dialogs plugin settings dataeditor core scriptbackends rbackend misc ${KDE4_KHTML_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KDEUI_LIBS})

# NOTE: KDE4_KFILE_LIBS still links against Qt3Support

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${DATA_INSTALL_DIR}/rkward
	PATTERN .svn EXCLUDE)

INSTALL(FILES   resource.ver rkwardui.rc rkconsolepart.rc DESTINATION ${DATA_INSTALL_DIR}/rkward )
INSTALL(TARGETS rkward.bin DESTINATION ${BIN_INSTALL_DIR})
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rkward DESTINATION ${BIN_INSTALL_DIR})

INSTALL(FILES rkward.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

