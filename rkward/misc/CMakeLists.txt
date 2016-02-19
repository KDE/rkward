INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )

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
   rkjobsequence.cpp
   rkspecialactions.cpp
   rkstandardicons.cpp
   rkstandardactions.cpp
   rkxmlguisyncer.cpp
   rkcommandhistory.cpp
   rktableview.cpp
   celleditor.cpp
   editlabelsdialog.cpp
   editformatdialog.cpp
   rkmessagecatalog.cpp
   rkdbusapi.cpp
   rkfindbar.cpp
   rkdynamicsearchline.cpp
   rkaccordiontable.cpp
   rkxmlguipreviewarea.cpp
   rkdialogbuttonbox.cpp
   )

ADD_LIBRARY(misc STATIC ${misc_STAT_SRCS})
TARGET_LINK_LIBRARIES(misc Qt5::Widgets KF5::WidgetsAddons KF5::KIOWidgets Qt5::Xml KF5::ConfigCore KF5::Parts KF5::WindowSystem)

########### install files ###############

INSTALL(FILES
	rkdummypart.rc
	DESTINATION ${KXMLGUI_INSTALL_DIR}/rkward)
