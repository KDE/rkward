INCLUDE(FindR.cmake)
IF(NOT WIN32)
	# for pthread_at_fork
	FIND_PACKAGE(Threads)
ENDIF(NOT WIN32)

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
TARGET_LINK_LIBRARIES(rbackend rkgraphicsdevice.frontend Qt5::Widgets KF5::TextEditor KF5::WindowSystem)

ADD_DEFINITIONS (-DRKWARD_BACKEND_PATH="${KDE_INSTALL_FULL_LIBEXECDIR}")
LINK_DIRECTORIES(${R_SHAREDLIBDIR})
ADD_EXECUTABLE(rkward.rbackend ${rbackend_backend_SRCS} ${rbackend_shared_SRCS})
ECM_MARK_NONGUI_EXECUTABLE(rkward.rbackend)
TARGET_LINK_LIBRARIES(rkward.rbackend rkgraphicsdevice.backend ${R_USED_LIBS} ${CMAKE_THREAD_LIBS_INIT} Qt5::Network Qt5::Core KF5::I18n)
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
    TARGET_LINK_LIBRARIES(rkward.rbackend "-framework CoreFoundation")
    ADD_CUSTOM_COMMAND(TARGET rkward.rbackend POST_BUILD
    COMMAND codesign -d --force --options runtime --verbose=4 -s - --entitlements "${CMAKE_CURRENT_SOURCE_DIR}/rkward.rbackend.entitlements" ${CMAKE_CURRENT_BINARY_DIR}/rkward.rbackend
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
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
	# on Widows, we install to the rbackend subdirectory, because 1) LIBEXEC_INSTALL_DIR == BIN_INSTALL_DIR and 2) we don't want the backend to pick up
	# all the KDE library versions, first, when loading DLLs
	INSTALL(TARGETS rkward.rbackend DESTINATION ${BIN_INSTALL_DIR}/rbackend)
ELSE(WIN32)
	INSTALL(TARGETS rkward.rbackend DESTINATION ${LIBEXEC_INSTALL_DIR})
ENDIF(WIN32)
