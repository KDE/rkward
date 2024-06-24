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
	rkpdfwindow.cpp
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
TARGET_LINK_LIBRARIES(windows Qt6::Widgets Qt6::PrintSupport Qt6::Core5Compat Qt6::WebEngineWidgets KF6::TextEditor KF6::Notifications KF6::WindowSystem KF6::KIOFileWidgets KF6::I18n)
