INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

KDE4_CREATE_HANDBOOK( index.docbook )
#KDE4_CREATE_HANDBOOK( writing_plugins_introduction.docbook )


########### install files ###############




#original Makefile.am contents follow:

#EXTRA_DIST = index.docbook writing_plugins_introduction.docbook menu_hierarchy_example.png \
#	t_test_plugin_example.png
#
#KDE_LANG = en
#KDE_DOCS = rkward
#
