IF (WIN32)
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
ENDIF (WIN32)
ADD_EXECUTABLE (rkward rkward_startup_wrapper.cpp rkward_windows_icon.rc)
add_definitions ("-DR_EXECUTABLE=\\\"${R_EXECUTABLE}\\\"")
TARGET_LINK_LIBRARIES(rkward ${QT_QTGUI_LIBRARY})
IF (WIN32)	# NOTE: On other platforms, currently in conflict with shell wrapper script
	INSTALL(TARGETS rkward DESTINATION ${BIN_INSTALL_DIR})
ENDIF (WIN32)
