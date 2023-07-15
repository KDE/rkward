# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

########### next target ###############

SET(windows_STAT_SRCS
	rkcommandeditorwindow.cpp
	rkdebugconsole.cpp
	rkcallstackviewer.cpp
	rkhtmlwindow.cpp
	rcontrolwindow.cpp
	detachedwindowcontainer.cpp
	rkmdiwindow.cpp
	rkworkplaceview.cpp
	rkworkplace.cpp
	rkwindowcatcher.cpp
	rkcommandlog.cpp
	rkhelpsearchwindow.cpp
	rktoplevelwindowgui.cpp
	rkfilebrowser.cpp
	rktoolwindowbar.cpp
	rktoolwindowlist.cpp
	robjectbrowser.cpp
	rkdebugmessagewindow.cpp
	katepluginintegration.cpp
	rkcodecompletion.cpp
	rktexthints.cpp
)

ADD_LIBRARY(windows STATIC ${windows_STAT_SRCS})
IF(NO_QT_WEBENGINE)
TARGET_LINK_LIBRARIES(windows Qt5::Widgets Qt5::Xml Qt5::PrintSupport Qt5::WebKitWidgets KF5::WebKit KF5::TextEditor KF5::Notifications KF5::WindowSystem KF5::KIOFileWidgets)
ELSE(NO_QT_WEBENGINE)
TARGET_LINK_LIBRARIES(windows Qt5::Widgets Qt5::Xml Qt5::PrintSupport Qt5::WebEngineWidgets KF5::TextEditor KF5::Notifications KF5::WindowSystem KF5::KIOFileWidgets)
ENDIF(NO_QT_WEBENGINE)
