# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
# Ok, so this seems a bit ove the top, and I hope we'll find an easier solution one of those days:
# During build, we package the support R package sources, so they can be installed in whatever R backend will be used at runtime.
# However, that breaks reproducible builds, as timestamps and owner info enter the tar. Those can be stripped with some extra options,
# but _only_ in GNU tar...
#
# The elaborate code below was taken from Extra CMake Modules / KDEPackageAppTemplates.cmake, where the same problem is faced.
find_program(_tar_executable NAMES gtar tar)
if(_tar_executable)
	execute_process(
		COMMAND ${_tar_executable} --sort=name --version
		TIMEOUT 3
		RESULT_VARIABLE _tar_exit
		OUTPUT_VARIABLE _tar_version
		ERROR_VARIABLE _tar_stderr
	)
	if("${_tar_exit}" EQUAL 0 AND "${_tar_version}" MATCHES "GNU tar")
		set(GNU_TAR_FOUND ON)
	else()
		set(GNU_TAR_FOUND OFF)
	endif()
else()
	set(GNU_TAR_FOUND OFF)
endif()


MACRO(createRpackage name)
	FILE(GLOB_RECURSE rkwardfiles${name} LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*")
        if(GNU_TAR_FOUND)
		# Honour SOURCE_DATE_EPOCH if set
		if(DEFINED ENV{SOURCE_DATE_EPOCH})
			set(TIMESTAMP $ENV{SOURCE_DATE_EPOCH})
		else()
		execute_process(
			COMMAND "date" "+%s"
			OUTPUT_VARIABLE TIMESTAMP
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		endif()

		# Make tar archive reproducible, the arguments are only available with GNU tar
		add_custom_command(OUTPUT ${name}.tgz
			COMMAND ${_tar_executable} ARGS
			cfz "${CMAKE_CURRENT_BINARY_DIR}/${name}.tgz" "${name}"
			--sort=name
			--numeric-owner --owner=0 --group=0
			--mtime="@${TIMESTAMP}"
			--pax-option=exthdr.name=%d/PaxHeaders/%f,delete=atime,delete=ctime
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${rkwardfiles${name}})
	ELSE(GNU_TAR_FOUND)
		ADD_CUSTOM_COMMAND(OUTPUT ${name}.tgz
			COMMAND ${CMAKE_COMMAND} -E tar "cfz" "${CMAKE_CURRENT_BINARY_DIR}/${name}.tgz" "${name}"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${rkwardfiles${name}})
	ENDIF(GNU_TAR_FOUND)
ENDMACRO()

createRpackage(rkward)
createRpackage(rkwardtests)
ADD_CUSTOM_TARGET(rpackages ALL DEPENDS rkward.tgz rkwardtests.tgz)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/rkward.tgz" "${CMAKE_CURRENT_BINARY_DIR}/rkwardtests.tgz" DESTINATION ${KDE_INSTALL_DATADIR}/rkward/rpackages)
