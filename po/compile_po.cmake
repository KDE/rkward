# helper script, which will compile a po into a gmo, if, and only if it is at least 80% translated.
# this is basically, because ADD_CUSTOM_COMMAND won't capture stdout, and EXECUTE_PROCESS
# can't be made to re-run when the po-file changes.
EXECUTE_PROCESS(COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile})	# For printing any errors / warnings
EXECUTE_PROCESS(COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --statistics -o ${_gmoFile} ${_poFile}	# Second run to grab stats
	ERROR_VARIABLE MSGFMT_STATS ERROR_STRIP_TRAILING_WHITESPACE)

# Try to extract statistics information on translated vs. untranslated strings
STRING(REGEX MATCHALL "[0-9]+"
	MSGFMT_COUNTS
	${MSGFMT_STATS})
LIST(LENGTH MSGFMT_COUNTS MSGFMT_COUNTS_COUNT)
IF(MSGFMT_COUNTS_COUNT LESS 2)
	LIST(APPEND MSGFMT_COUNTS 0)		# This means X translated, and no untranslated strings
ENDIF(MSGFMT_COUNTS_COUNT LESS 2)
IF(MSGFMT_COUNTS_COUNT LESS 3)
	LIST(INSERT MSGFMT_COUNTS 1 0)		# This means X translated, no fuzzy, Y untranslated strings
ENDIF(MSGFMT_COUNTS_COUNT LESS 3)
LIST(GET MSGFMT_COUNTS 0 TRANSLATED_COUNT)
LIST(GET MSGFMT_COUNTS 2 UNTRANSLATED_COUNT)
MATH(EXPR TRANSLATION_RATIO "${TRANSLATED_COUNT}*100/(${TRANSLATED_COUNT}+${UNTRANSLATED_COUNT})")

# Purge hopelessly incomplete translations
IF(TRANSLATION_RATIO LESS 80)
	MESSAGE (STATUS "${_poFile} is only ${TRANSLATION_RATIO}% translated. Will not be installed.")
	FILE(REMOVE ${_gmoFile})
ELSE(TRANSLATION_RATIO LESS 80)
	MESSAGE (STATUS "${_poFile} is ${TRANSLATION_RATIO}% translated.")
ENDIF(TRANSLATION_RATIO LESS 80)
