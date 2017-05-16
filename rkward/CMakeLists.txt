IF(NOT ${PROJECT_NAME} STREQUAL "rkward")
	MESSAGE (FATAL_ERROR "Build triggered from wrong subdirectory. Please point cmake to the parent of this directory, instead.")
ENDIF(NOT ${PROJECT_NAME} STREQUAL "rkward")

INCLUDE(SetVersionNumber.cmake)

IF(Q_WS_MAC)
	SET(CPACK_BUNDLE_ICON rkward.icns)
	SET(CPACK_BUNDLE_NAME RKWard)
	SET(CPACK_BUNDLE_STARTUP_COMMAND rkward)
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
ADD_SUBDIRECTORY( windows )
ADD_SUBDIRECTORY( syntax )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}   ${QT_QTWEBKIT_INCLUDE_DIR} )

SET(RKWard_Sources
	rkward.cpp
	main.cpp
	rkglobals.cpp
	robjectviewer.cpp
	rkconsole.cpp
)

# somehow the detected R paths from FindR.cmake do not get passed down automatically
GET_DIRECTORY_PROPERTY(R_EXECUTABLE DIRECTORY rbackend DEFINITION R_EXECUTABLE)
GET_DIRECTORY_PROPERTY(R_SHAREDLIBDIR DIRECTORY rbackend LINK_DIRECTORIES)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

ADD_CUSTOM_COMMAND (OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rkward.ico"
			COMMAND cmake -E copy "${CMAKE_CURRENT_SOURCE_DIR}/icons/app-icon/rkward.ico"
			"${CMAKE_CURRENT_BINARY_DIR}/rkward.ico")

ADD_EXECUTABLE(rkward ${RKWard_Sources} rkward_windows_icon.rc rkward.ico)
# NOTE: These definitions are needed for the startup procedure (main.cpp), only.
# We should switch with to target_compile_definitions once we require CMAKE 2.6+
ADD_DEFINITIONS(-DR_EXECUTABLE="${R_EXECUTABLE}")
ADD_DEFINITIONS(-DINSTALL_PATH="${CMAKE_INSTALL_PREFIX}")
ADD_DEFINITIONS(-DR_LIBS="${R_LIBDIR}")
SET_TARGET_PROPERTIES(rkward PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "RKWard")

IF(Q_WS_MAC)
	SET(RKWARD_INFOPLIST_TEMPLATE Info.plist.in)
	SET(RKWARD_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/rkward/Info.plist)
	CONFIGURE_FILE(
		${RKWARD_INFOPLIST_TEMPLATE}
		${RKWARD_INFOPLIST_FILE}
		@ONLY)
ENDIF(Q_WS_MAC)

TARGET_LINK_LIBRARIES(rkward windows ${RKWARD_ADDLIBS} agents dialogs plugin settings dataeditor core scriptbackends rbackend misc KF5::WindowSystem Qt5::Widgets KF5::XmlGui ${LibIntl_LIBRARIES})
IF(KF5Crash_FOUND)
	TARGET_LINK_LIBRARIES(rkward KF5::Crash)
	SET_SOURCE_FILES_PROPERTIES(main.cpp PROPERTIES COMPILE_DEFINITIONS WITH_KCRASH=1)
ENDIF(KF5Crash_FOUND)

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${DATA_INSTALL_DIR}/rkward
	PATTERN .svn EXCLUDE)

INSTALL(FILES rkwardui.rc rkconsolepart.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/rkward)
INSTALL(FILES resource.ver DESTINATION ${DATA_INSTALL_DIR}/rkward)
INSTALL(TARGETS rkward DESTINATION ${BIN_INSTALL_DIR})

IF(Q_WS_MAC)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/rkward/icons/app-icon/rkward.icns DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents/Resources)
	INSTALL(FILES ${RKWARD_INFOPLIST_FILE} DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents)
ENDIF(Q_WS_MAC)

INSTALL(FILES org.kde.rkward.desktop org.kde.rkward-open.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES vnd.rkward.r.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
INSTALL(FILES rkward.protocol DESTINATION ${SERVICES_INSTALL_DIR})
