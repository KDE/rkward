# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )

SET(core_STAT_SRCS
   rkvariable.cpp
   robjectlist.cpp
   robject.cpp
   rcontainerobject.cpp
   rkmodificationtracker.cpp
   rfunctionobject.cpp
   renvironmentobject.cpp
   rkrownames.cpp
   rkpseudoobjects.cpp
   )

ADD_LIBRARY(core STATIC ${core_STAT_SRCS})
TARGET_LINK_LIBRARIES(core Qt5::Core Qt5::Widgets KF5::WidgetsAddons KF5::I18n KF5::Parts)

