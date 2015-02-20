MACRO(RKHandlePO _PO_FILES)
	FOREACH(_poFile ${_PO_FILES})
		GET_FILENAME_COMPONENT(_fullPoName ${_poFile} NAME)
		STRING(REPLACE "." ";" _nameParts ${_fullPoName})
		LIST(LENGTH _nameParts _namePartsCount)
		LIST(GET _nameParts 0 _poid)
		LIST(GET _nameParts 1 _lang) # Remainder of _nameParts should be "po"
		IF(NOT ${_poid} STREQUAL "rkward")
			SET(ACCEPT_INCOMPLETE_PO "-DACCEPT_INCOMPLETE_PO=1")
		ELSE(NOT ${_poid} STREQUAL "rkward")
			SET(ACCEPT_INCOMPLETE_PO "")
		ENDIF(NOT ${_poid} STREQUAL "rkward")

		SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_poid}.${_lang}.gmo)

		# making the gmo be re-built, when the po changes, but not every time is surprsingly difficult
		# (since the gmo file is only built for translations which are complete enough)
		SET(_stampFile ${CMAKE_CURRENT_BINARY_DIR}/${_poid}.${_lang}.stamp)
		ADD_CUSTOM_COMMAND(OUTPUT ${_stampFile}
			COMMAND ${CMAKE_COMMAND} "-D_poFile=${_poFile}" "-D_gmoFile=${_gmoFile}" "-DGETTEXT_MSGFMT_EXECUTABLE=${GETTEXT_MSGFMT_EXECUTABLE}" ${ACCEPT_INCOMPLETE_PO} -P ${CMAKE_CURRENT_SOURCE_DIR}/compile_po.cmake
			COMMAND ${CMAKE_COMMAND} -E touch ${_stampFile}
			COMMENT "Generating translation for language '${_lang}', catalog '${_poid}'"
			DEPENDS ${_poFile})
		LIST(APPEND active_translations ${_stampFile})

		IF(${_poid} STREQUAL "rkward")
			INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_MESSAGES/ RENAME rkward.mo OPTIONAL)
		ELSE(${_poid} STREQUAL "rkward")
			INSTALL(FILES ${_gmoFile} DESTINATION ${DATA_INSTALL_DIR}/rkward/po/${_lang}/LC_MESSAGES/ RENAME ${_poid}.mo OPTIONAL)
		ENDIF(${_poid} STREQUAL "rkward")
	ENDFOREACH(_poFile ${_PO_FILES})
ENDMACRO(RKHandlePO)


FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
	MESSAGE(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)
	IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/")
		FILE(GLOB PO_FILES po/*.po)
		RKHandlePO("${PO_FILES}")

		ADD_CUSTOM_TARGET(translations ALL DEPENDS ${active_translations})
	ELSE(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/")
	MESSAGE(WARNING
"------
                 Translations are not available.

${CMAKE_CURRENT_SOURCE_DIR}/po/ was not found. This probably means that you are building from the development repository, and have not fetched translations. This is ok, if you want to run RKWard in English, only. Otherwise, to fetch translations, either
   scripts/import_translations.py XX
where (XX is your language code, such as \"de\"; optionally specify several codes separated by spaces). In some cases (esp., if you want to build all existing translations), it is much faster to use:
   git clone git://anongit.kde.org/scratch/tfry/rkward-po-export i18n/po
------")
	ENDIF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/")
ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)

