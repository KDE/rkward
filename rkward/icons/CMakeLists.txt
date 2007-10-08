INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL(FILES   rkward_logo.png function.png list.png window_back.png window_forward.png run_line.png run_selection.png run_all.png DESTINATION ${DATA_INSTALL_DIR}/rkward/icons )

KDE4_INSTALL_ICONS( hicolor )



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#KDE_ICON = rkward
#iconsdir = $(kde_datadir)/rkward/icons
#
#icons_DATA = rkward_logo.png function.png list.png window_back.png window_forward.png run_line.png run_selection.png run_all.png
#
