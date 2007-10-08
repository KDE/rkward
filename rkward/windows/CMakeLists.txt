INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(windows_STAT_SRCS
   rkcommandeditorwindow.cpp
   rkcommandeditorwindowpart.cpp
   rkhtmlwindow.cpp
   rcontrolwindow.cpp
   detachedwindowcontainer.cpp
   rkmdiwindow.cpp
   rkworkplaceview.cpp
   rkworkplace.cpp
   qxembedcopy.cpp
   rkwindowcatcher.cpp
   rkcommandlog.cpp
   rkhelpsearchwindow.cpp
   rktoplevelwindowgui.cpp
   rkfilebrowser.cpp
   )

QT4_AUTOMOC(${windows_STAT_SRCS})

ADD_LIBRARY(windows STATIC ${windows_STAT_SRCS})


########### install files ###############

INSTALL(FILES   rkcommandeditorwindowpart.rc rkoutputwindow.rc rkhelpwindow.rc detachedwindowcontainer.rc  	rkcatchedx11windowpart.rc rkcommandlogpart.rc rktoplevelwindowgui.rc DESTINATION /share/apps/rkward )



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#noinst_LIBRARIES =  libwindows.a
#noinst_HEADERS = rkcommandeditorwindow.h rkcommandeditorwindowpart.h \
#	rkhtmlwindow.h rcontrolwindow.h detachedwindowcontainer.h rkmdiwindow.h \
#	rkworkplaceview.h rkworkplace.h qxembedcopy.h rkwindowcatcher.h rkcommandlog.h \
#	rkhelpsearchwindow.h rktoplevelwindowgui.h rkfilebrowser.h
#libwindows_a_SOURCES = rkcommandeditorwindow.cpp rkcommandeditorwindowpart.cpp \
#	rkhtmlwindow.cpp rcontrolwindow.cpp detachedwindowcontainer.cpp rkmdiwindow.cpp \
#	rkworkplaceview.cpp rkworkplace.cpp qxembedcopy.cpp rkwindowcatcher.cpp rkcommandlog.cpp \
#	rkhelpsearchwindow.cpp rktoplevelwindowgui.cpp rkfilebrowser.cpp
#rcdir = $(kde_datadir)/rkward
#rc_DATA = rkcommandeditorwindowpart.rc rkoutputwindow.rc rkhelpwindow.rc detachedwindowcontainer.rc \
#	rkcatchedx11windowpart.rc rkcommandlogpart.rc rktoplevelwindowgui.rc
