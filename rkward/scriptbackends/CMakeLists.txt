# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )

SET(scriptbackends_STAT_SRCS
   scriptbackend.cpp
   simplebackend.cpp
   qtscriptbackend.cpp
   qtscripti18n.cpp
   rkcomponentscripting.cpp
   )

ADD_LIBRARY(scriptbackends STATIC ${scriptbackends_STAT_SRCS})
TARGET_LINK_LIBRARIES(scriptbackends Qt6::Qml Qt6::Widgets Qt6::Xml KF6::WidgetsAddons KF6::I18n)

INSTALL(FILES common.js rkcomponentscripting.js DESTINATION ${KDE_INSTALL_DATADIR}/rkward/phpfiles )
