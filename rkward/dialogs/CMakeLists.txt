# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )

SET(dialogs_STAT_SRCS
   rkloadlibsdialog.cpp
   rkreadlinedialog.cpp
   rkimportdialog.cpp
   rkselectlistdialog.cpp
   rkrecoverdialog.cpp
   rkerrordialog.cpp
   rksavemodifieddialog.cpp
   rksetupwizard.cpp
   )

ADD_DEFINITIONS(-DLIBEXECDIR="${KDE_INSTALL_LIBEXECDIR}")
ADD_LIBRARY(dialogs STATIC ${dialogs_STAT_SRCS})
TARGET_LINK_LIBRARIES(dialogs Qt6::Widgets KF6::Parts KF6::ConfigWidgets KF6::TextEditor KF6::I18n KF6::KIOWidgets)
