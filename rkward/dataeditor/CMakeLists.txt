INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(dataeditor_STAT_SRCS
   twintable.cpp
   twintablemember.cpp
   rkdrag.cpp
   rkeditor.cpp
   rkeditordataframe.cpp
   twintabledatamember.cpp
   twintablemetamember.cpp
   celleditor.cpp
   editlabelsdialog.cpp
   editformatdialog.cpp
   rkeditordataframepart.cpp
   )

QT4_AUTOMOC(${dataeditor_STAT_SRCS})

ADD_LIBRARY(dataeditor STATIC ${dataeditor_STAT_SRCS})


########### install files ###############

INSTALL(FILES   rkeditordataframepart.rc DESTINATION /share/apps/rkward )



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libdataeditor.a
#libdataeditor_a_SOURCES = twintable.cpp twintablemember.cpp rkdrag.cpp \
#	rkeditor.cpp rkeditordataframe.cpp twintabledatamember.cpp twintablemetamember.cpp \
#	celleditor.cpp editlabelsdialog.cpp editformatdialog.cpp rkeditordataframepart.cpp
#noinst_HEADERS = rkdrag.h twintable.h twintablemember.h rkeditor.h \
#		rkeditordataframe.h twintabledatamember.h twintablemetamember.h celleditor.h \
#		editlabelsdialog.h editformatdialog.h rkeditordataframepart.h
#
#rcdir = $(kde_datadir)/rkward
#rc_DATA = rkeditordataframepart.rc
