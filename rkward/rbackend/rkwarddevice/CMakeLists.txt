# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE_DIRECTORIES( ${R_INCLUDEDIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}    )

SET (
	rkgraphicsdevice_backend_SRCS
	rkgraphicsdevice_backendtransmitter.cpp
	rkgraphicsdevice_setup.cpp
#	rkgraphicsdevice_stubs.cpp is included by rkgraphicsdevice_setup.cpp
)

SET (
	rkgraphicsdevice_frontend_SRCS
	rkgraphicsdevice_frontendtransmitter.cpp
	rkgraphicsdevice.cpp
)

ADD_LIBRARY(rkgraphicsdevice.frontend STATIC ${rkgraphicsdevice_frontend_SRCS})
TARGET_LINK_LIBRARIES(rkgraphicsdevice.frontend Qt6::Widgets Qt6::Core Qt6::Network KF6::CoreAddons KF6::WidgetsAddons KF6::I18n KF6::ConfigCore KF6::Parts)

ADD_LIBRARY(rkgraphicsdevice.backend STATIC ${rkgraphicsdevice_backend_SRCS})
TARGET_LINK_LIBRARIES(rkgraphicsdevice.backend Qt6::Core Qt6::Network)
IF(${DLOPEN_RLIB})
  TARGET_COMPILE_DEFINITIONS(rkgraphicsdevice.backend PUBLIC RK_DLOPEN_LIBRSO)
  IF(NOT WIN32)
    TARGET_COMPILE_OPTIONS(rkgraphicsdevice.backend PUBLIC -fPIC)
  ENDIF()
ENDIF()
