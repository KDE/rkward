INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

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
	rkwindowcatcher.cpp
	rkcommandlog.cpp
	rkhelpsearchwindow.cpp
	rktoplevelwindowgui.cpp
	rkfilebrowser.cpp
	rktoolwindowbar.cpp
)

QT4_AUTOMOC(${windows_STAT_SRCS})

ADD_LIBRARY(windows STATIC ${windows_STAT_SRCS})


########### install files ###############

INSTALL(FILES
	rkcommandeditorwindowpart.rc
	rkoutputwindow.rc
	rkhelpwindow.rc
	detachedwindowcontainer.rc
	rkcatchedx11windowpart.rc
	rkcommandlogpart.rc
	rktoplevelwindowgui.rc
	DESTINATION ${DATA_INSTALL_DIR}/rkward)
