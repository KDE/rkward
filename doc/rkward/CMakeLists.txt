KDE4_CREATE_HANDBOOK (index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en)

ADD_CUSTOM_TARGET (webdocs
	COMMAND meinproc4 --stylesheet ${DATA_INSTALL_DIR}/ksgmltools2/customization/kde-web.xsl ${CMAKE_CURRENT_SOURCE_DIR}/writing_plugins_introduction.docbook
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
