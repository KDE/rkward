# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(misc_STAT_SRCS
   rkspinbox.cpp
   getfilenamewidget.cpp
   rkobjectlistview.cpp
   xmlhelper.cpp
   multistringselector.cpp
   rkcommandlineargs.cpp
   rkcommonfunctions.cpp
   rkprogresscontrol.cpp
   rksaveobjectchooser.cpp
   rkdummypart.cpp
   rkjobsequence.cpp
   rkspecialactions.cpp
   rkstandardicons.cpp
   rkstandardactions.cpp
   rkxmlguisyncer.cpp
   rkcommandhistory.cpp
   rktableview.cpp
   celleditor.cpp
   editlabelsdialog.cpp
   editformatdialog.cpp
   rkmessagecatalog.cpp
   rkfindbar.cpp
   rkdynamicsearchline.cpp
   rkaccordiontable.cpp
   rkxmlguipreviewarea.cpp
   rkdialogbuttonbox.cpp
   rkoutputdirectory.cpp
   rkstyle.cpp
   rkparsedversion.cpp
   )

ADD_LIBRARY(misc STATIC ${misc_STAT_SRCS})
TARGET_LINK_LIBRARIES(misc Qt6::Widgets KF6::WidgetsAddons KF6::KIOWidgets Qt6::Xml Qt6::DBus KF6::ConfigCore KF6::Parts KF6::WindowSystem KF6::TextEditor KF6::Archive KF6::I18n)
