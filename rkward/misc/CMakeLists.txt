INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}    )


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
   )

ADD_LIBRARY(misc STATIC ${misc_STAT_SRCS})
TARGET_LINK_LIBRARIES(misc Qt5::Widgets KF5::KDELibs4Support)

########### install files ###############

INSTALL(FILES
	rkdummypart.rc
	DESTINATION ${KXMLGUI_INSTALL_DIR}/rkward)
