INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(plugin_STAT_SRCS
   rkcomponentmap.cpp
   rkcomponentmeta.cpp
   rkcomponentproperties.cpp
   rkcomponent.cpp
   rkstandardcomponent.cpp
   rkvarselector.cpp
   rkvarslot.cpp
   rkformula.cpp
   rkradio.cpp
   rkcheckbox.cpp
   rkpluginspinbox.cpp
   rkinput.cpp
   rkpluginbrowser.cpp
   rktext.cpp
   rktabpage.cpp
   rkstandardcomponentgui.cpp
   rkdropdown.cpp
   rkcomponentcontext.cpp
   rkpreviewbox.cpp
   rkpluginsaveobject.cpp
   rkabstractoptionselector.cpp
   rkpluginframe.cpp
   rkoptionset.cpp
   rkmatrixinput.cpp
   )

QT4_AUTOMOC(${plugin_STAT_SRCS})

ADD_LIBRARY(plugin STATIC ${plugin_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libplugin.a
#libplugin_a_SOURCES = rkcomponentmap.cpp rkcomponentproperties.cpp rkcomponent.cpp \
#	rkstandardcomponent.cpp rkvarselector.cpp rkvarslot.cpp rkformula.cpp rkradio.cpp \
#	rkcheckbox.cpp rkpluginspinbox.cpp rkinput.cpp rkpluginbrowser.cpp rktext.cpp \
#	rktabpage.cpp rkstandardcomponentgui.cpp rkdropdown.cpp rkcomponentcontext.cpp \
#	rkpreviewbox.cpp rkpluginsaveobject.cpp rkabstractoptionselector.cpp
#
#noinst_HEADERS = rkcomponentmap.h rkcomponentproperties.h rkcomponent.h \
#	rkstandardcomponent.h rkvarselector.h rkvarslot.h rkformula.h rkradio.h \
#	rkcheckbox.h rkpluginspinbox.h rkinput.h rkpluginbrowser.h rktext.h \
#	rktabpage.h rkstandardcomponentgui.h rkdropdown.h rkcomponentcontext.h \
#	rkpreviewbox.h rkpluginsaveobject.h rkabstractoptionselector.h
#
#
