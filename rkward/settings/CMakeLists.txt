# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )


########### next target ###############

SET(settings_STAT_SRCS
   rksettings.cpp
   rksettingsmodule.cpp
   rksettingsmodulegeneral.cpp
   rksettingsmoduleoutput.cpp
   rksettingsmodulegraphics.cpp
   rksettingsmodulekateplugins.cpp
   rksettingsmoduleplugins.cpp
   rksettingsmoduler.cpp
   rksettingsmodulewatch.cpp
   rksettingsmoduleobjectbrowser.cpp
   rksettingsmoduleconsole.cpp
   rksettingsmodulecommandeditor.cpp
   rksettingsmoduledebug.cpp
   rkrecenturls.cpp
   )

IF (WIN32 OR USE_BINARY_PACKAGES)
	SET_PROPERTY (
		SOURCE rksettingsmoduler.cpp
		APPEND
		PROPERTY COMPILE_DEFINITIONS
		USE_BINARY_PACKAGES=1
	)
ENDIF (WIN32 OR USE_BINARY_PACKAGES)

ADD_LIBRARY(settings STATIC ${settings_STAT_SRCS})
TARGET_LINK_LIBRARIES(settings Qt6::Widgets KF6::WidgetsAddons Qt6::Xml KF6::ConfigCore KF6::Parts KF6::TextEditor KF6::I18n KF6::KIOWidgets)
