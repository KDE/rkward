# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(dataeditor_STAT_SRCS
   twintable.cpp
   twintablemember.cpp
   rkeditor.cpp
   rkeditordataframe.cpp
   rkvareditmodel.cpp
   rktextmatrix.cpp
   )

ADD_LIBRARY(dataeditor STATIC ${dataeditor_STAT_SRCS})
TARGET_LINK_LIBRARIES(dataeditor Qt6::Widgets KF6::WidgetsAddons KF6::Parts KF6::I18n)
