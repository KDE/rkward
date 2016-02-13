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
)

ADD_LIBRARY(windows STATIC ${windows_STAT_SRCS})
TARGET_LINK_LIBRARIES(windows Qt5::Widgets Qt5::Xml Qt5::PrintSupport KF5::TextEditor KF5::WebKit KF5::Notifications KF5::WindowSystem KF5::KIOFileWidgets)

########### install files ###############

INSTALL(FILES
	rkcommandeditorwindowpart.rc
	rkoutputwindow.rc
	rkhelpwindow.rc
	detachedwindowcontainer.rc
	rkcatchedx11windowpart.rc
	rkcommandlogpart.rc
	rktoplevelwindowgui.rc
	rkstandardactions.rc
	DESTINATION ${KXMLGUI_INSTALL_DIR}/rkward)
