# NOTE: kdoctools_create_handbook does not support multiple docbooks inside one directory. That's why this is separate.
# KF5 TODO: Is the above still true?
kdoctools_create_handbook (index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR rkwardplugins)

ADD_CUSTOM_TARGET (webdocs
	COMMAND meinproc4 --stylesheet ${DATA_INSTALL_DIR}/ksgmltools2/customization/kde-web.xsl ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
