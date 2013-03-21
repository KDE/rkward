INCLUDE_DIRECTORIES( ${R_INCLUDEDIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

SET (
	rkgraphicsdevice_backend_SRCS
	rkgraphicsdevice_backendtransmitter.cpp
	rkgraphicsdevice_setup.cpp
#	rkgraphicsdevice_stubs.cpp is included by rkgraphicsdevice_setup.cpp
)

SET (
	rkgraphicsdevice_frontend_SRCS
	rkgraphicsdevice_frontendtransmitter.cpp
)

QT4_AUTOMOC(${rkgraphicsdevice_frontend_SRCS})
ADD_LIBRARY(rkgraphicsdevice.frontend STATIC ${rkgraphicsdevice_frontend_SRCS})

QT4_AUTOMOC(${rkgraphicsdevice_backend_SRCS})
ADD_LIBRARY(rkgraphicsdevice.backend STATIC ${rkgraphicsdevice_backend_SRCS})
