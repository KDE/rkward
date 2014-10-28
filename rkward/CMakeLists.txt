IF(NOT ${PROJECT_NAME} STREQUAL "rkward")
	MESSAGE (FATAL_ERROR "Build triggered from wrong subdirectory. Please point cmake to the parent of this directory, instead.")
ENDIF(NOT ${PROJECT_NAME} STREQUAL "rkward")

INCLUDE(SetVersionNumber.cmake)

IF(Q_WS_MAC)
	SET(RKWARD_FRONTEND_LOCATION ${BIN_INSTALL_DIR}/rkward.frontend.app/Contents/MacOS)
	SET(CPACK_BUNDLE_ICON rkward.icns)
	SET(CPACK_BUNDLE_NAME RKWard)
	SET(CPACK_BUNDLE_STARTUP_COMMAND rkward)
ELSE(Q_WS_MAC)
	SET(RKWARD_FRONTEND_LOCATION ${LIBEXEC_INSTALL_DIR})
ENDIF(Q_WS_MAC)

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
IF(WIN32)
	ADD_SUBDIRECTORY( qwinhost )
	LIST(APPEND RKWARD_ADDLIBS qwinhost)
ELSE(WIN32)
	LIST(APPEND RKWARD_ADDLIBS ${X11_X11_LIB})
ENDIF(WIN32)
ADD_SUBDIRECTORY( windows )
ADD_SUBDIRECTORY( syntax )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${X11_X11_INCLUDE_PATH} )

SET(RKWard_Sources
	rkward.cpp
	main.cpp
	rkglobals.cpp
	robjectviewer.cpp
	rkconsole.cpp
	rkwardapplication.cpp
)

# auto-gen sources
#QT4_AUTOMOC(${RKWard_Sources})	# not needed here, as KDE4_ADD_EXECUTABLE mocs the sources in this dir.

# somehow the needed libdirs for R do not get passed down automatically
GET_DIRECTORY_PROPERTY(R_SHAREDLIBDIR DIRECTORY rbackend LINK_DIRECTORIES)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

KDE4_ADD_EXECUTABLE(rkward.frontend ${RKWard_Sources})
SET_TARGET_PROPERTIES(rkward.frontend PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "RKWard")

IF(Q_WS_MAC)
	SET(RKWARD_INFOPLIST_TEMPLATE Info.plist.in)
	SET(RKWARD_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/rkward/Info.plist)
	CONFIGURE_FILE(
		${RKWARD_INFOPLIST_TEMPLATE}
		${RKWARD_INFOPLIST_FILE}
		@ONLY)
ENDIF(Q_WS_MAC)

TARGET_LINK_LIBRARIES(rkward.frontend ${KDE4_KDECORE_LIBS} windows ${RKWARD_ADDLIBS} agents dialogs plugin settings dataeditor core scriptbackends rbackend misc ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTSCRIPT_LIBRARY} ${QT_QTNETWORK_LIBRARY})

# wrapper executable
GET_DIRECTORY_PROPERTY(R_EXECUTABLE DIRECTORY rbackend DEFINITION R_EXECUTABLE)
IF (WIN32)
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
ENDIF (WIN32)
ADD_CUSTOM_COMMAND (OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rkward.ico"
			COMMAND cmake -E copy "${CMAKE_CURRENT_SOURCE_DIR}/icons/app-icon/rkward.ico"
			"${CMAKE_CURRENT_BINARY_DIR}/rkward.ico")
ADD_EXECUTABLE (rkward rkward_startup_wrapper.cpp rkward_windows_icon.rc rkward.ico)
# NOTE: These definitions are needed for the wrapper, only.
# We should switch with to target_compile_definitions once we require CMAKE 2.6+
add_definitions ("-DR_EXECUTABLE=\\\"${R_EXECUTABLE}\\\"")
add_definitions ("-DINSTALL_PATH=\\\"${CMAKE_INSTALL_PREFIX}\\\"")
add_definitions ("-DR_LIBS=\\\"${R_LIBDIR}\\\"")
add_definitions ("-DRKWARD_FRONTEND_LOCATION=\\\"${RKWARD_FRONTEND_LOCATION}\\\"")
TARGET_LINK_LIBRARIES(rkward ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

ADD_DEPENDENCIES(rkward.frontend rkward)

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${DATA_INSTALL_DIR}/rkward
	PATTERN .svn EXCLUDE)

INSTALL(FILES resource.ver rkwardui.rc rkconsolepart.rc DESTINATION ${DATA_INSTALL_DIR}/rkward )

IF(Q_WS_MAC)
	INSTALL(TARGETS rkward.frontend DESTINATION ${BIN_INSTALL_DIR})
	INSTALL(TARGETS rkward DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents/MacOS)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/rkward/icons/app-icon/rkward.icns DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents/Resources)
	INSTALL(FILES ${RKWARD_INFOPLIST_FILE} DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents)
ELSE(Q_WS_MAC)
	INSTALL(TARGETS rkward.frontend DESTINATION ${LIBEXEC_INSTALL_DIR})
	INSTALL(TARGETS rkward DESTINATION ${BIN_INSTALL_DIR})
ENDIF(Q_WS_MAC)

INSTALL(FILES rkward.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
