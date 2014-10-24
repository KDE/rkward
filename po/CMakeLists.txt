MACRO(RKHandlePO _PO_FILES)
	FOREACH(_poFile ${_PO_FILES})
		GET_FILENAME_COMPONENT(_fullPoName ${_poFile} NAME)
		STRING(REPLACE "." ";" _nameParts ${_fullPoName})
		LIST(LENGTH _nameParts _namePartsCount)
		IF(_namePartsCount GREATER 2)
			LIST(GET _nameParts 0 _poid)
			LIST(REMOVE_AT _nameParts 0)
		ELSE(_namePartsCount GREATER 2)
			SET(_poid "rkward")
		ENDIF(_namePartsCount GREATER 2)
		LIST(GET _nameParts 0 _lang) # Remainder of _nameParts should be "po"

		SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poid}.${_lang}.gmo)

		# making the gmo be re-built, when the po changes, but not every time is surprsingly difficult
		# (since the gmo file is only built for translations which are complete enough)
		SET(_stampFile ${CMAKE_CURRENT_BINARY_DIR}/${_poid}.${_lang}.stamp)
		ADD_CUSTOM_COMMAND(OUTPUT ${_stampFile}
			COMMAND ${CMAKE_COMMAND} "-D_poFile=${_poFile}" "-D_gmoFile=${_gmoFile}" "-DGETTEXT_MSGFMT_EXECUTABLE=${GETTEXT_MSGFMT_EXECUTABLE}" -P ${CMAKE_CURRENT_SOURCE_DIR}/compile_po.cmake
			COMMAND ${CMAKE_COMMAND} -E touch ${_stampFile}
			COMMENT "Generating translation for language '${_lang}', catalog '${_poid}'"
			DEPENDS ${_poFile})
		LIST(APPEND active_translations ${_stampFile})

		IF(_poid STREQUAL "rkward")
			INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_MESSAGES/ RENAME rkward.mo OPTIONAL)
		ELSE(_poid STREQUAL "rkward")
			INSTALL(FILES ${_gmoFile} DESTINATION ${DATA_INSTALL_DIR}/rkward/po/${_lang}/LC_MESSAGES/ RENAME ${_poid}.mo OPTIONAL)
		ENDIF(_poid STREQUAL "rkward")
	ENDFOREACH(_poFile ${_PO_FILES})
ENDMACRO(RKHandlePO)


FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
	MESSAGE(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)
	FILE(GLOB PO_FILES *.po)
	RKHandlePO("${PO_FILES}")
	FILE(GLOB_RECURSE PLUGIN_PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/plugins/*.po)
	RKHandlePO("${PLUGIN_PO_FILES}")

	ADD_CUSTOM_TARGET(translations ALL DEPENDS ${active_translations})
ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)

