# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
CMAKE_MINIMUM_REQUIRED(VERSION 3.20.0)

PROJECT(rkward)

IF(NOT CMAKE_VERBOSE_MAKEFILE)
	SET (FORCE_PRETTY_MAKEFILE ON)
ENDIF(NOT CMAKE_VERBOSE_MAKEFILE)

FIND_PACKAGE(ECM 6.0.0 REQUIRED NO_MODULE)

SET(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

INCLUDE(KDEInstallDirs)
INCLUDE(KDECMakeSettings)
INCLUDE(KDECompilerSettings)
INCLUDE(KDEClangFormat)
FILE(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.js)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
INCLUDE(KDEGitCommitHooks)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
INCLUDE(ECMInstallIcons)
INCLUDE(ECMAddAppIcon)
INCLUDE(ECMMarkNonGuiExecutable)
INCLUDE(FeatureSummary)

OPTION(FORCE_WITH_QWEBENGINE "Build QWebEngine based HTML renderer (overrides automatic selection)" OFF)
ADD_FEATURE_INFO("FORCE_WITH_QWEBENGINE: Build QWebEngine based HTML renderer" FORCE_WITH_QWEBENGINE "Override automatic detection of with HTML render to build for. Setting this option to ON implies that no other HTML renderer support is compiled, unless it is also specified with the corresponding option (see FORCE_WITH_QWEBVIEW).")

OPTION(FORCE_WITH_QWEBVIEW "Build QWebView based HTML renderer (overrides automatic selection)" OFF)
ADD_FEATURE_INFO("FORCE_WITH_QWEBVIEW: Build QWebView based HTML renderer" FORCE_WITH_QWEBVIEW "Override automatic detection of with HTML render to build for. Setting this option to ON implies that no other HTML renderer support is compiled, unless it is also specified with the corresponding option (see FORCE_WITH_QWEBENGINE).")

OPTION(DLOPEN_RLIB "dlopen the R library at runtime rather than linking it" ON)
ADD_FEATURE_INFO("DLOPEN_RLIB: dlopen() R library" DLOPEN_RLIB "This feature allows more flexibility in using a different R installation at runtime than at compile time.")

SET(REQUIRED_QT6_LIBS Widgets Core Xml Network Qml PrintSupport)
SET(OPTIONAL_QT6_LIBS)
IF(FORCE_WITH_QWEBENGINE)
  LIST(APPEND REQUIRED_QT6_LIBS WebEngineWidgets)
ELSE()
  LIST(APPEND OPTIONAL_QT6_LIBS WebEngineWidgets)
ENDIF()
IF(FORCE_WITH_QWEBVIEW)
  LIST(APPEND REQUIRED_QT6_LIBS WebView WebSockets)
ELSE()
  LIST(APPEND OPTIONAL_QT6_LIBS WebView WebSockets)
ENDIF()

FIND_PACKAGE(Qt6 6.6 CONFIG REQUIRED COMPONENTS ${REQUIRED_QT6_LIBS} OPTIONAL_COMPONENTS ${OPTIONAL_QT6_LIBS})
FIND_PACKAGE(KF6 6.0.0 REQUIRED COMPONENTS CoreAddons DocTools I18n XmlGui TextEditor WidgetsAddons Parts Config Notifications WindowSystem Archive BreezeIcons OPTIONAL_COMPONENTS Crash)
FIND_PACKAGE(Gettext REQUIRED)

if(Qt6WebEngineWidgets_FOUND AND (${FORCE_WITH_QWEBENGINE} OR NOT ${FORCE_WITH_QWEBVIEW}))
  set(WITH_QWEBENGINE ON)
  set(RK_QWEBENGINE_FLAGS -DRK_WITH_QWEBENGINE=1)
else()
  set(WITH_QWEBENGINE OFF)
  set(RK_QWEBENGINE_FLAGS -DRK_WITH_QWEBENGINE=0)
endif()
if(Qt6WebView_FOUND AND Qt6WebSockets_FOUND AND (${FORCE_WITH_QWEBVIEW} OR NOT ${FORCE_WITH_QWEBENGINE}))
  set(WITH_QWEBVIEW ON)
  set(RK_QWEBVIEW_FLAGS -DRK_WITH_QWEBVIEW=1)
else()
  set(WITH_QWEBVIEW OFF)
  set(RK_QWEBVIEW_FLAGS -DRK_WITH_QWEBVIEW=0)
endif()

if(NOT WIN32) # will apparently pick up trash, on Windows, sometimes
  FIND_PACKAGE(KDSingleApplication-qt6 1.1.0 QUIET)
endif()
if(KDSingleApplication-qt6_FOUND)
  message(STATUS "Using system KDSingleApplication")
else()
  message(STATUS "Using bundled KDSingleApplication")
  add_subdirectory(3rdparty/KDSingleApplication)
  add_definitions(-DKDSINGLEAPPLICATION_STATIC_BUILD)
endif()

IF(FORCE_PRETTY_MAKEFILE)
	SET(CMAKE_VERBOSE_MAKEFILE OFF)
ENDIF(FORCE_PRETTY_MAKEFILE)

ADD_DEFINITIONS(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

#uncomment the line below to save ~250-350kB in object size
#ADD_DEFINITIONS(-DRKWARD_NO_TRACE)

ADD_SUBDIRECTORY(rkward)
ADD_SUBDIRECTORY(doc)
ADD_SUBDIRECTORY(tests)
KI18N_INSTALL(po)
KDOCTOOLS_INSTALL(po)

FEATURE_SUMMARY(WHAT ENABLED_FEATURES DISABLED_FEATURES)
FEATURE_SUMMARY(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
message(STATUS "QWebView support: ${WITH_QWEBVIEW}")
message(STATUS "QWebEngine support: ${WITH_QWEBENGINE}")
if(NOT (WITH_QWEBVIEW OR WITH_QWEBENGINE))
  message(FATAL_ERROR "Found no suitable HTML renderer. Please install either the QWebEngineWidgets library or QWebView and QWebSockets.")
endif()
