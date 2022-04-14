IF(NOT ${PROJECT_NAME} STREQUAL "rkward")
	MESSAGE (FATAL_ERROR "Build triggered from wrong subdirectory. Please point cmake to the parent of this directory, instead.")
ENDIF(NOT ${PROJECT_NAME} STREQUAL "rkward")

INCLUDE(SetVersionNumber.cmake)

IF(APPLE)
	SET(CPACK_BUNDLE_ICON rkward.icns)
	SET(CPACK_BUNDLE_NAME RKWard)
	SET(CPACK_BUNDLE_STARTUP_COMMAND rkward)
ENDIF()

# once tested and working this could become the default on Mac:
OPTION(APPLE_STANDALONE_BUNDLE "Generate a standalone app bundle when building on Mac" OFF)
ADD_FEATURE_INFO("App Bundle" APPLE_STANDALONE_BUNDLE "Generate a standalone app bundle when building on Mac. Do not use for now.")

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

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(RKWard_Sources
	rkward.cpp
	main.cpp
	robjectviewer.cpp
	rkconsole.cpp
)
QT5_ADD_RESOURCES(RKWard_Sources resources.qrc)

# somehow the detected R paths from FindR.cmake do not get passed down automatically
GET_DIRECTORY_PROPERTY(R_EXECUTABLE DIRECTORY rbackend DEFINITION R_EXECUTABLE)
GET_DIRECTORY_PROPERTY(R_SHAREDLIBDIR DIRECTORY rbackend LINK_DIRECTORIES)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

FILE(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/app-icon/*-apps-rkward.png")
# recent ECM versions will prefer the scalable version: prepend it to the icon source list (older versions generate a warning on the SVG, but so what):
ECM_ADD_APP_ICON(RKWard_Sources ICONS ${CMAKE_CURRENT_SOURCE_DIR}/icons/app-icon/sc-apps-rkward.svgz ${ICON_SRCS})
ADD_EXECUTABLE(rkward ${RKWard_Sources})

# NOTE: These definitions are needed for the startup procedure (main.cpp), only.
# We should switch with to target_compile_definitions once we require CMAKE 2.6+
ADD_DEFINITIONS(-DR_EXECUTABLE="${R_EXECUTABLE}")
ADD_DEFINITIONS(-DINSTALL_PATH="${CMAKE_INSTALL_PREFIX}")
SET_TARGET_PROPERTIES(rkward PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "RKWard")

IF(APPLE)
	SET(RKWARD_INFOPLIST_TEMPLATE Info.plist.in)
	SET(RKWARD_INFOPLIST_FILE ${CMAKE_SOURCE_DIR}/rkward/Info.plist)
	CONFIGURE_FILE(
		${RKWARD_INFOPLIST_TEMPLATE}
		${RKWARD_INFOPLIST_FILE}
		@ONLY)
ENDIF(APPLE)

TARGET_LINK_LIBRARIES(rkward windows ${RKWARD_ADDLIBS} agents dialogs plugin settings dataeditor core scriptbackends rbackend misc KF5::WindowSystem Qt5::Widgets KF5::XmlGui)
IF(KF5Crash_FOUND)
	TARGET_LINK_LIBRARIES(rkward KF5::Crash)
	SET_SOURCE_FILES_PROPERTIES(main.cpp PROPERTIES COMPILE_DEFINITIONS WITH_KCRASH=1)
ENDIF(KF5Crash_FOUND)

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${DATA_INSTALL_DIR}/rkward)
INSTALL(FILES resource.ver DESTINATION ${DATA_INSTALL_DIR}/rkward)
IF(NOT WIN32)
	# This is for running directly from the build tree. Not on windows for now (until cmake supports symlinks, there)
	ADD_CUSTOM_TARGET(local_install ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/pages ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/pages
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/plugins ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/plugins
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/resource.ver ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/resource.ver
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/scriptbackends ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/phpfiles
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/rbackend/rpackages ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/rpackages
	)
ENDIF(NOT WIN32)


IF(APPLE)
    INSTALL(TARGETS rkward DESTINATION ${BUNDLE_INSTALL_DIR})
    if(APPLE_STANDALONE_BUNDLE)
#         INSTALL(FILES ${CMAKE_SOURCE_DIR}/rkward/icons/app-icon/rkward.icns DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents/Resources)
#         INSTALL(FILES ${RKWARD_INFOPLIST_FILE} DESTINATION ${BUNDLE_INSTALL_DIR}/${CPACK_BUNDLE_NAME}.app/Contents)
    ENDIF()
    # the MacOSX bundle display name property (CFBundleDisplayName) is not currently supported by cmake,
    # so has to be set for all targets in this cmake file
    SET(MACOSX_BUNDLE_DISPLAY_NAME RKWard)
    # We provide our own Info.plist, but for the wrapper only
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${RKWARD_INFOPLIST_FILE})
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.RKWard")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "RKWard")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_DISPLAY_NAME "RKWard")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_INFO_STRING "Frontend to the R statistics language")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING "RKWard ${RKVERSION_NUMBER}")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "${RKVERSION_NUMBER}")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION "${RKVERSION_NUMBER}")
    set_target_properties(rkward PROPERTIES MACOSX_BUNDLE_COPYRIGHT "RKWard Team")
ELSE()
    INSTALL(TARGETS rkward DESTINATION ${BIN_INSTALL_DIR})
ENDIF()

INSTALL(FILES org.kde.rkward.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES org.kde.rkward.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
INSTALL(FILES vnd.rkward.r.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
INSTALL(FILES rkward.protocol DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL(FILES RKWardRMd.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
