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

FIND_PACKAGE(Qt6 6.6 CONFIG REQUIRED COMPONENTS Widgets Core Xml Network Qml PrintSupport WebEngineWidgets WebView)
FIND_PACKAGE(KF6 6.0.0 REQUIRED COMPONENTS CoreAddons DocTools I18n XmlGui TextEditor WidgetsAddons Parts Config Notifications WindowSystem Archive BreezeIcons OPTIONAL_COMPONENTS Crash)
FIND_PACKAGE(Gettext REQUIRED)

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

FEATURE_SUMMARY(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
