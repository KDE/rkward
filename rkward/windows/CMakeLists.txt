INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

########### next target ###############

SET(windows_STAT_SRCS
	rkcommandeditorwindow.cpp
	rkdebugconsole.cpp
	rkcallstackviewer.cpp
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
	rktoolwindowlist.cpp
	robjectbrowser.cpp
	rkdebugmessagewindow.cpp
	katepluginintegration.cpp
	rkcodecompletion.cpp
)

ADD_LIBRARY(windows STATIC ${windows_STAT_SRCS})
TARGET_LINK_LIBRARIES(windows Qt5::Widgets Qt5::Xml Qt5::PrintSupport Qt5::WebKitWidgets KF5::TextEditor KF5::WebKit KF5::Notifications KF5::WindowSystem KF5::KIOFileWidgets)
