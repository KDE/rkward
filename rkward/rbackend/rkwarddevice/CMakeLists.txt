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
TARGET_LINK_LIBRARIES(rkgraphicsdevice.frontend Qt5::Widgets Qt5::Core Qt5::Network KF5::WidgetsAddons KF5::KDELibs4Support)

ADD_LIBRARY(rkgraphicsdevice.backend STATIC ${rkgraphicsdevice_backend_SRCS})
