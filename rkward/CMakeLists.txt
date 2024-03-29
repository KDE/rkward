# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
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
IF(BUILD_TESTING)
	ADD_SUBDIRECTORY(autotests)
ENDIF(BUILD_TESTING)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(RKWard_Sources
	rkward.cpp
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
ADD_LIBRARY(rkward_lib STATIC ${RKWard_Sources}) # For auto-tests: pack everything except main in a dummy library
TARGET_COMPILE_DEFINITIONS(rkward_lib PUBLIC -DR_EXECUTABLE="${R_EXECUTABLE}")
ADD_EXECUTABLE(rkward main.cpp)

# NOTE: These definitions are needed for the startup procedure (main.cpp), only.
# We should switch with to target_compile_definitions once we require CMAKE 2.6+
TARGET_COMPILE_DEFINITIONS(rkward PUBLIC -DINSTALL_PATH="${CMAKE_INSTALL_PREFIX}")
FILE(RELATIVE_PATH RelPathToData /${BIN_INSTALL_DIR} /${DATA_INSTALL_DIR})
TARGET_COMPILE_DEFINITIONS(rkward PUBLIC -DREL_PATH_TO_DATA="${RelPathToData}")

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

TARGET_LINK_LIBRARIES(rkward_lib windows ${RKWARD_ADDLIBS} agents dialogs plugin settings dataeditor core scriptbackends rbackend misc KF5::WindowSystem Qt5::Widgets KF5::XmlGui)
TARGET_LINK_LIBRARIES(rkward rkward_lib)
IF(KF5Crash_FOUND)
	TARGET_LINK_LIBRARIES(rkward KF5::Crash)
	SET_SOURCE_FILES_PROPERTIES(main.cpp PROPERTIES COMPILE_DEFINITIONS WITH_KCRASH=1)
ENDIF(KF5Crash_FOUND)

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${DATA_INSTALL_DIR}/rkward)
INSTALL(FILES resource.ver DESTINATION ${DATA_INSTALL_DIR}/rkward)
# This is for running directly from the build tree.
EXECUTE_PROCESS(
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/rbackend/rpackages # populated later during the build
)
EXECUTE_PROCESS(
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/icons
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/pages ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/pages
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/plugins ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/plugins
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/resource.ver ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/resource.ver
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/scriptbackends ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/phpfiles
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/rbackend/rpackages ${CMAKE_CURRENT_BINARY_DIR}/rkwardinstall/rpackages
	RESULT_VARIABLE SYMLINKS_FAILED
)
IF(SYMLINKS_FAILED)
	MESSAGE(WARNING "Failed to create symbolic links inside build tree. This is ok in a regular installation, but running from the build tree will fail (i.e. tests will fail).")
ENDIF(SYMLINKS_FAILED)


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
INSTALL(FILES org.kde.rkward.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
INSTALL(FILES rkward.protocol DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL(FILES vnd.rkward.r.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
INSTALL(FILES vnd.kde.rkward-output.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
INSTALL(FILES vnd.kde.rmarkdown.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
FIND_PACKAGE(SharedMimeInfo)
IF(SharedMimeInfo_FOUND)
	UPDATE_XDG_MIMETYPES(${XDG_MIME_INSTALL_DIR})
ENDIF(SharedMimeInfo_FOUND)

INSTALL(FILES RKWardRMd.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardLaTeX.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardR.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardroxygen2.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardutils.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardpapaja.xml DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_snippets/data)
