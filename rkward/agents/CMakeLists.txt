# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )


########### next target ###############

SET(agents_STAT_SRCS
   rkdebughandler.cpp
   rkeditobjectagent.cpp
   rkloadagent.cpp
   rkprintagent.cpp
   rkquitagent.cpp
   rksaveagent.cpp
   showedittextfileagent.cpp
   )

ADD_LIBRARY(agents STATIC ${agents_STAT_SRCS})
TARGET_LINK_LIBRARIES(agents Qt6::Widgets KF6::TextEditor KF6::I18n KF6::Service KF6::KIOCore KF6::KIOWidgets)
