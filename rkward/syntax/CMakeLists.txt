INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES   r.xml rkward.xml DESTINATION ${DATA_INSTALL_DIR}/katepart/syntax/ )

# r-syntax-test.R is not installed
