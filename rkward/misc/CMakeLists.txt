INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(misc_STAT_SRCS
   rkspinbox.cpp
   getfilenamewidget.cpp
   rkobjectlistview.cpp
   xmlhelper.cpp
   multistringselector.cpp
   rkcommonfunctions.cpp
   rkprogresscontrol.cpp
   rksaveobjectchooser.cpp
   rkdummypart.cpp
   rkstandardicons.cpp
   )

QT4_AUTOMOC(${misc_STAT_SRCS})

ADD_LIBRARY(misc STATIC ${misc_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libmisc.a
#libmisc_a_SOURCES = rkspinbox.cpp \
#	getfilenamewidget.cpp rkobjectlistview.cpp rkcanceldialog.cpp xmlhelper.cpp \
#	multistringselector.cpp rkcommonfunctions.cpp rkprogresscontrol.cpp rksaveobjectchooser.cpp \
#	rkdummypart.cpp
#noinst_HEADERS = rkspinbox.h getfilenamewidget.h \
#	rkobjectlistview.h rkcanceldialog.h xmlhelper.h multistringselector.h \
#	rkcommonfunctions.h rkprogresscontrol.h rksaveobjectchooser.h \
#	rkdummypart.h
#
