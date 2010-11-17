INCLUDE(FindR.cmake)
IF(NOT WIN32)
	FIND_PACKAGE(Threads)
	INCLUDE(DetectPthreads.cmake)
ENDIF(NOT WIN32)

ADD_SUBDIRECTORY( rpackages )

INCLUDE_DIRECTORIES( ${R_INCLUDEDIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

SET (
	rbackend_shared_SRCS
	rkrbackendprotocol_shared.cpp
	rdata.cpp
)

SET (
	rbackend_backend_SRCS
	rkrbackend.cpp
	rkpthreadsupport.cpp
	rksignalsupport.cpp
	rklocalesupport.cpp
	rkrsupport.cpp
	rkstructuregetter.cpp
	rkrbackendprotocol_backend.cpp
)

SET (
	rbackend_frontend_SRCS
	rinterface.cpp
	rcommand.cpp
	rcommandreceiver.cpp
	rcommandstack.cpp
	rkrbackendprotocol_frontend.cpp
)

SET(RBACKEND_SPLIT 1)
IF(RBACKEND_SPLIT)
	SET (
		rbackend_STAT_SRCS
		${rbackend_frontend_SRCS}
		${rbackend_shared_SRCS}
		rkfrontendtransmitter.cpp
	)
	QT4_AUTOMOC(${rbackend_STAT_SRCS})
	ADD_LIBRARY(rbackend STATIC ${rbackend_STAT_SRCS})
	TARGET_LINK_LIBRARIES(rbackend ${CMAKE_THREAD_LIBS_INIT})

	QT4_AUTOMOC(${rbackend_backend_SRCS})
	ADD_DEFINITIONS (-DRKWARD_SPLIT_PROCESS)
	KDE4_ADD_EXECUTABLE(rkward.rbackend ${rbackend_backend_SRCS} ${rbackend_shared_SRCS})
	TARGET_LINK_LIBRARIES(rkward.rbackend ${R_USED_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${KDE4_KDECORE_LIBS})
	LINK_DIRECTORIES(${R_SHAREDLIBDIR})
ELSE(RBACKEND_SPLIT)
	SET (
		rbackend_STAT_SRCS
		${rbackend_frontend_SRCS}
		${rbackend_shared_SRCS}
		${rbackend_backend_SRCS}
	)

	QT4_AUTOMOC(${rbackend_STAT_SRCS})
	ADD_LIBRARY(rbackend STATIC ${rbackend_STAT_SRCS})
	TARGET_LINK_LIBRARIES(rbackend ${R_USED_LIBS} ${CMAKE_THREAD_LIBS_INIT})
	LINK_DIRECTORIES(${R_SHAREDLIBDIR})
ENDIF(RBACKEND_SPLIT)
