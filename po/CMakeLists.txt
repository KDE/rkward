FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
	MESSAGE(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)

	FILE(GLOB PO_FILES *.po)

	FOREACH(_poFile ${PO_FILES})
		GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
		SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

		# making the gmo be re-built, when the po changes, but not every time is surprsingly difficult
		# (since the gmo file is only built for translations which are complete enough)
		SET(_stampFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.stamp)
		ADD_CUSTOM_COMMAND(OUTPUT ${_stampFile}
			COMMAND ${CMAKE_COMMAND} "-D_poFile=${_poFile}" "-D_gmoFile=${_gmoFile}" "-DGETTEXT_MSGFMT_EXECUTABLE=${GETTEXT_MSGFMT_EXECUTABLE}" -P ${CMAKE_CURRENT_SOURCE_DIR}/compile_po.cmake
			COMMAND ${CMAKE_COMMAND} -E touch ${_stampFile}
			COMMENT "Generating translation for language '${_lang}'"
			DEPENDS ${_poFile})
		LIST(APPEND active_translations ${_stampFile})

		INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_MESSAGES/ RENAME rkward.mo OPTIONAL)
	ENDFOREACH(_poFile ${PO_FILES})

	ADD_CUSTOM_TARGET(translations ALL DEPENDS ${active_translations})

ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)
