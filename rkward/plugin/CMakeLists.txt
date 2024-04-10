# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )


########### next target ###############

SET(plugin_STAT_SRCS
   rkcomponentmap.cpp
   rkcomponentmeta.cpp
   rkcomponentproperties.cpp
   rkcomponent.cpp
   rkstandardcomponent.cpp
   rkvarselector.cpp
   rkvarslot.cpp
   rkformula.cpp
   rkradio.cpp
   rkcheckbox.cpp
   rkpluginspinbox.cpp
   rkinput.cpp
   rkpluginbrowser.cpp
   rktext.cpp
   rktabpage.cpp
   rkstandardcomponentgui.cpp
   rkdropdown.cpp
   rkcomponentcontext.cpp
   rkpreviewbox.cpp
   rkpluginsaveobject.cpp
   rkabstractoptionselector.cpp
   rkpluginframe.cpp
   rkoptionset.cpp
   rkmatrixinput.cpp
   rkvalueselector.cpp
   )

ADD_LIBRARY(plugin STATIC ${plugin_STAT_SRCS})
TARGET_LINK_LIBRARIES(plugin Qt6::Widgets KF6::TextEditor KF6::WindowSystem KF6::I18n)
