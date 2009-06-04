IF(WIN32)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

	# QT4-automoc does not work, as qwinhost.cpp does not have the #include "qwhinhost.moc"-line.
	# We don't want to add it to keep changes small, and use manual moccing, here, instead
	QT4_WRAP_CPP(MANUAL_MOC_CXX qwinhost.h)
	SET(qwinhost_STAT_SRCS
		qwinhost.cpp
		${MANUAL_MOC_CXX}
	)

	ADD_LIBRARY(qwinhost STATIC ${qwinhost_STAT_SRCS})
ELSE(WIN32)
	MESSAGE(STATUS "Only needed on windows")
ENDIF(WIN32)
