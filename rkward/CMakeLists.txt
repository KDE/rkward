# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
IF(NOT ${PROJECT_NAME} STREQUAL "rkward")
	MESSAGE (FATAL_ERROR "Build triggered from wrong subdirectory. Please point cmake to the parent of this directory, instead.")
ENDIF(NOT ${PROJECT_NAME} STREQUAL "rkward")

INCLUDE(SetVersionNumber.cmake)

OPTION(DLOPEN_RLIB "dlopen the R library at runtime rather than linking it" ON)
ADD_FEATURE_INFO("dlopen() R library" DLOPEN_RLIB "This feature allows more flexibility in using a different R installation at runtime than at compile time.")

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
IF(BUILD_TESTING)
	ADD_SUBDIRECTORY(autotests)
ENDIF(BUILD_TESTING)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# For auto-tests: pack everything except main in a dummy library
SET(RKWard_Lib_Sources
	rkward.cpp
	robjectviewer.cpp
	rkconsole.cpp
)
QT_ADD_RESOURCES(RKWard_Lib_Sources resources.qrc icons/icons.qrc)

# somehow the detected R paths from FindR.cmake do not get passed down automatically
GET_DIRECTORY_PROPERTY(R_SHAREDLIBDIR DIRECTORY rbackend LINK_DIRECTORIES)
LINK_DIRECTORIES(${R_SHAREDLIBDIR})

ADD_LIBRARY(rkward_lib STATIC ${RKWard_Lib_Sources})
TARGET_LINK_LIBRARIES(rkward_lib windows ${RKWARD_ADDLIBS} agents dialogs plugin settings dataeditor core scriptbackends rbackend misc KDAB::kdsingleapplication KF6::WindowSystem Qt6::Widgets KF6::XmlGui KF6::BreezeIcons)

SET(RKWard_App_Sources
	main.cpp
)
FILE(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/app-icon/*-apps-rkward.png")
# recent ECM versions will prefer the scalable version: prepend it to the icon source list (older versions generate a warning on the SVG, but so what):
ECM_ADD_APP_ICON(RKWard_App_Sources ICONS ${CMAKE_CURRENT_SOURCE_DIR}/icons/app-icon/sc-apps-rkward.svgz ${ICON_SRCS})
ADD_EXECUTABLE(rkward ${RKWard_App_Sources})

# NOTE: These path definitions are needed for the startup procedure (main.cpp), only.
TARGET_COMPILE_DEFINITIONS(rkward PUBLIC -DINSTALL_PATH="${CMAKE_INSTALL_PREFIX}")
FILE(RELATIVE_PATH RelPathToData /${KDE_INSTALL_BINDIR} /${KDE_INSTALL_DATADIR})
TARGET_COMPILE_DEFINITIONS(rkward PUBLIC -DREL_PATH_TO_DATA="${RelPathToData}")
TARGET_LINK_LIBRARIES(rkward rkward_lib)
IF(KF6Crash_FOUND)
	TARGET_LINK_LIBRARIES(rkward KF6::Crash)
	SET_SOURCE_FILES_PROPERTIES(main.cpp PROPERTIES COMPILE_DEFINITIONS WITH_KCRASH=1)
ENDIF(KF6Crash_FOUND)

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

########### install files ###############

INSTALL(DIRECTORY plugins/ pages DESTINATION ${KDE_INSTALL_DATADIR}/rkward)
INSTALL(FILES resource.ver DESTINATION ${KDE_INSTALL_DATADIR}/rkward)
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
ENDIF()
INSTALL(TARGETS rkward ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES org.kde.rkward.desktop DESTINATION ${KDE_INSTALL_APPDIR})
INSTALL(FILES org.kde.rkward.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
INSTALL(FILES rkward.protocol DESTINATION ${KDE_INSTALL_DATADIR}/kio/servicemenus)
INSTALL(FILES vnd.rkward.r.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
INSTALL(FILES vnd.kde.rkward-output.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
INSTALL(FILES vnd.kde.rmarkdown.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
FIND_PACKAGE(SharedMimeInfo)
IF(SharedMimeInfo_FOUND)
	UPDATE_XDG_MIMETYPES(${KDE_INSTALL_MIMEDIR})
ENDIF(SharedMimeInfo_FOUND)

INSTALL(FILES RKWardRMd.xml DESTINATION ${KDE_INSTALL_DATADIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardLaTeX.xml DESTINATION ${KDE_INSTALL_DATADIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardR.xml DESTINATION ${KDE_INSTALL_DATADIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardroxygen2.xml DESTINATION ${KDE_INSTALL_DATADIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardutils.xml DESTINATION ${KDE_INSTALL_DATADIR}/ktexteditor_snippets/data)
INSTALL(FILES RKWardpapaja.xml DESTINATION ${KDE_INSTALL_DATADIR}/ktexteditor_snippets/data)
