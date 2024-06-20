# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
INCLUDE(FindR.cmake)
IF(NOT WIN32)
	# for pthread_at_fork
	FIND_PACKAGE(Threads)
ENDIF(NOT WIN32)

SET(DLOPEN_RLIB 1) # TODO: Allow configuration
ADD_SUBDIRECTORY( rpackages )
ADD_SUBDIRECTORY( rkwarddevice )

INCLUDE_DIRECTORIES( ${R_INCLUDEDIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET (
	rbackend_shared_SRCS
	rkrbackendprotocol_shared.cpp
	rdata.cpp
	rktransmitter.cpp
)

SET (
	rbackend_backend_SRCS
	rkrbackend.cpp
	rksignalsupport.cpp
	rkrsupport.cpp
	rkstructuregetter.cpp
	rkrbackendprotocol_backend.cpp
	rkreventloop.cpp
	rkbackendtransmitter.cpp
	rkrapi.cpp
)

SET (
	rbackend_frontend_SRCS
	rkrinterface.cpp
	rcommand.cpp
	rcommandstack.cpp
	rkrbackendprotocol_frontend.cpp
	rksessionvars.cpp
	rkfrontendtransmitter.cpp
)

ADD_LIBRARY(rbackend STATIC ${rbackend_frontend_SRCS} ${rbackend_shared_SRCS})
SET(REL_PATH_TO_LIBEXEC ${KDE_INSTALL_LIBEXECDIR})
CMAKE_PATH(RELATIVE_PATH REL_PATH_TO_LIBEXEC BASE_DIRECTORY ${KDE_INSTALL_BINDIR})
TARGET_COMPILE_DEFINITIONS(rbackend PUBLIC -DREL_PATH_TO_LIBEXEC="${REL_PATH_TO_LIBEXEC}")
TARGET_COMPILE_DEFINITIONS(rbackend PUBLIC -DR_EXECUTABLE="${R_EXECUTABLE}")
TARGET_LINK_LIBRARIES(rbackend rkgraphicsdevice.frontend Qt6::Widgets KF6::TextEditor KF6::WindowSystem)

ADD_DEFINITIONS (-DRKWARD_BACKEND_PATH="${KDE_INSTALL_FULL_LIBEXECDIR}")

# See rkrapi.h for documentation and rationale of the linking strategy
IF(${DLOPEN_RLIB})
  ADD_LIBRARY(rkward.rbackend.lib SHARED ${rbackend_backend_SRCS} ${rbackend_shared_SRCS})
  TARGET_LINK_LIBRARIES(rkward.rbackend.lib rkgraphicsdevice.backend ${CMAKE_THREAD_LIBS_INIT} Qt6::Network Qt6::Core KF6::I18n)
  TARGET_COMPILE_DEFINITIONS(rkward.rbackend.lib PUBLIC RK_DLOPEN_LIBRSO)
  TARGET_COMPILE_DEFINITIONS(rbackend PUBLIC RK_DLOPEN_LIBRSO)
  SET(REL_PATH_TO_LIB ${KDE_INSTALL_LIBDIR})
  CMAKE_PATH(RELATIVE_PATH REL_PATH_TO_LIB BASE_DIRECTORY ${KDE_INSTALL_BINDIR})
  TARGET_COMPILE_DEFINITIONS(rbackend PUBLIC -DREL_PATH_TO_LIB="${REL_PATH_TO_LIB}")
  INSTALL(TARGETS rkward.rbackend.lib DESTINATION ${KDE_INSTALL_LIBDIR})
  IF(APPLE)
    TARGET_LINK_LIBRARIES(rkward.rbackend.lib "-framework CoreFoundation")
  ENDIF()

  ADD_EXECUTABLE(rkward.rbackend rkrbackend_dlopen.cpp)
  IF(NOT WIN32)
    TARGET_LINK_LIBRARIES(rkward.rbackend dl)
  ENDIF()
ELSE(${DLOPEN_RLIB})
  LINK_DIRECTORIES(${R_SHAREDLIBDIR})
  ADD_EXECUTABLE(rkward.rbackend ${rbackend_backend_SRCS} ${rbackend_shared_SRCS})
  TARGET_LINK_LIBRARIES(rkward.rbackend rkgraphicsdevice.backend ${R_USED_LIBS} ${CMAKE_THREAD_LIBS_INIT} Qt6::Network Qt6::Core KF6::I18n)
ENDIF(${DLOPEN_RLIB})
ECM_MARK_NONGUI_EXECUTABLE(rkward.rbackend)

IF(WIN32)
# 64MB stack size is what R itself is build with on Windows, while by default the stack size would be 1MB
# Not sure, if 64MB is actually needed, but 1MB does cause trouble e.g. with some shiny apps.
    IF(MSVC)
        SET_TARGET_PROPERTIES(rkward.rbackend PROPERTIES LINK_FLAGS /STACK:64000000)
    ELSE(MSVC)
        SET_TARGET_PROPERTIES(rkward.rbackend PROPERTIES LINK_FLAGS -Wl,--stack,64000000)
    ENDIF(MSVC)
ENDIF(WIN32)

IF(APPLE)
    IF(NOT ${DLOPEN_RLIB})
        TARGET_LINK_LIBRARIES(rkward.rbackend "-framework CoreFoundation")
    ENDIF()
    ADD_CUSTOM_COMMAND(TARGET rkward.rbackend POST_BUILD
    COMMAND codesign -d --force --options runtime --verbose=4 -s - --entitlements "${CMAKE_CURRENT_SOURCE_DIR}/rkward.rbackend.entitlements" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rkward.rbackend
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Adding entitlment for rkward.rbackend to link to external R lib...")
ENDIF()
IF(MSVC)
# When compiling with MSVC, we have to create a .lib file for R.dll, first
FIND_PACKAGE(Python3 COMPONENTS Interpreter REQUIRED)
ADD_CUSTOM_TARGET(R_lib COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/GenLibFile.py ${LIBR_SO} ${CMAKE_BINARY_DIR} ${CMAKE_SYSTEM_PROCESSOR}
                        COMMENT "Generate .lib file for R.dll")
ADD_DEPENDENCIES(rkward.rbackend R_lib)
ENDIF(MSVC)

IF(WIN32)
	# on Windows, we install to the rbackend subdirectory, because 1) LIBEXEC_INSTALL_DIR == BIN_INSTALL_DIR and 2) we don't want the backend to pick up
	# all the KDE library versions, first, when loading DLLs
	INSTALL(TARGETS rkward.rbackend DESTINATION ${KDE_INSTALL_BINDIR}/rbackend)
ELSE(WIN32)
	INSTALL(TARGETS rkward.rbackend DESTINATION ${KDE_INSTALL_LIBEXECDIR})
ENDIF(WIN32)
