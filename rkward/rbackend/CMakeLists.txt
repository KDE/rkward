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
)

SET (
	rbackend_backend_SRCS
	rkrbackend.cpp
	rksignalsupport.cpp
	rkrsupport.cpp
	rkstructuregetter.cpp
	rkrbackendprotocol_backend.cpp
	rkreventloop.cpp
)

SET (
	rbackend_frontend_SRCS
	rkrinterface.cpp
	rcommand.cpp
	rcommandreceiver.cpp
	rcommandstack.cpp
	rkrbackendprotocol_frontend.cpp
	rksessionvars.cpp
)

SET (
	rbackend_FRONTEND_SRCS
	${rbackend_frontend_SRCS}
	${rbackend_shared_SRCS}
	rkfrontendtransmitter.cpp
	rktransmitter.cpp
)
ADD_LIBRARY(rbackend STATIC ${rbackend_FRONTEND_SRCS})
TARGET_LINK_LIBRARIES(rbackend rkgraphicsdevice.frontend Qt5::Widgets KF5::TextEditor KF5::WindowSystem)

SET (
	rbackend_BACKEND_SRCS
	${rbackend_backend_SRCS}
	${rbackend_shared_SRCS}
	rkbackendtransmitter.cpp
	rktransmitter.cpp
)
ADD_DEFINITIONS (-DRKWARD_SPLIT_PROCESS)
ADD_DEFINITIONS (-DRKWARD_BACKEND_PATH="${KDE_INSTALL_FULL_LIBEXECDIR}")
LINK_DIRECTORIES(${R_SHAREDLIBDIR})
ADD_EXECUTABLE(rkward.rbackend ${rbackend_BACKEND_SRCS})
ECM_MARK_NONGUI_EXECUTABLE(rkward.rbackend)
TARGET_LINK_LIBRARIES(rkward.rbackend rkgraphicsdevice.backend ${R_USED_LIBS} ${CMAKE_THREAD_LIBS_INIT} Qt5::Network Qt5::Core KF5::I18n)
IF(APPLE)
    TARGET_LINK_LIBRARIES(rkward.rbackend "-framework CoreFoundation")
ENDIF()

IF(WIN32)
	# on Widows, we install to the rbackend subdirectory, because 1) LIBEXEC_INSTALL_DIR == BIN_INSTALL_DIR and 2) we don't want the backend to pick up
	# all the KDE library versions, first, when loading DLLs
	INSTALL(TARGETS rkward.rbackend DESTINATION ${BIN_INSTALL_DIR}/rbackend)
ELSE(WIN32)
	INSTALL(TARGETS rkward.rbackend DESTINATION ${LIBEXEC_INSTALL_DIR})
ENDIF(WIN32)
