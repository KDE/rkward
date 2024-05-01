# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: LGPL-2.1-or-later

# find the R binary

MESSAGE(STATUS "Looking for R executable")
IF(R_EXECUTABLE)
	MESSAGE(STATUS "Specified by user")
ENDIF(R_EXECUTABLE)
SET(CMAKE_FIND_APPBUNDLE NEVER)  # Do not get fooled by R GUI on Mac
FIND_PROGRAM(R_EXECUTABLE R PATH_SUFFIXES lib/R/bin)

IF(R_EXECUTABLE)
	MESSAGE(STATUS "Using R at ${R_EXECUTABLE}")
ELSE(R_EXECUTABLE)
	MESSAGE(FATAL_ERROR "Could NOT find R (if R is installed but no in path, call cmake with -DR_ECEUTABLE=/path/to/R)")
ENDIF(R_EXECUTABLE)

IF(NOT NO_CHECK_R)
    # find out about R architecture (needed for some paths)
    EXECUTE_PROCESS(
        COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "--no-init-file" "-e" "cat(R.version$arch)"
        OUTPUT_VARIABLE R_ARCH)
        IF (${R_ARCH} STREQUAL "x86_64")
            SET (R_ARCH "x64")
        ENDIF (${R_ARCH} STREQUAL "x86_64")
    MESSAGE (STATUS "R architecture is ${R_ARCH}")

    # check R version.
    SET (R_MIN_VERSION "2.10.0")
    MESSAGE (STATUS "Checking R version")
    EXECUTE_PROCESS(
        COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "--no-init-file" "-e" "cat (paste(R.version$major, R.version$minor, sep='.'))"
        OUTPUT_VARIABLE R_VERSION)
    MESSAGE (STATUS "R version is ${R_VERSION}")
    EXECUTE_PROCESS(
        COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "--no-init-file" "-e" "min_ver <- '${R_MIN_VERSION}'; if (compareVersion ('${R_VERSION}', min_ver) < 0) cat ('At least R version', min_ver, 'is required')"
        OUTPUT_VARIABLE R_VERSION_STATUS)
    IF (R_VERSION_STATUS)
        MESSAGE (FATAL_ERROR ${R_VERSION_STATUS})
    ENDIF (R_VERSION_STATUS)
ENDIF(NOT NO_CHECK_R)

# find R_HOME

MESSAGE(STATUS "Looking for R_HOME")
IF(NOT R_HOME)
	EXECUTE_PROCESS(
		COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "--no-init-file" "-e" "cat(R.home())"
		OUTPUT_VARIABLE R_HOME)
ELSE(NOT R_HOME)
	MESSAGE(STATUS "Specified by user")
ENDIF(NOT R_HOME)
IF(NOT R_HOME)
	MESSAGE(FATAL_ERROR "Could NOT determine R_HOME (probably you misspecified the location of R)")
ELSE(NOT R_HOME)
	MESSAGE(STATUS "R_HOME is ${R_HOME}")
ENDIF(NOT R_HOME)

# find R include dir

MESSAGE(STATUS "Looking for R include files")
IF(NOT R_INCLUDEDIR)
	IF(WIN32 OR APPLE)	# This version of the test will not work with R < 2.9.0, but the other version (in the else part) will not work on windows or apple (but we do not really need to support ancient versions of R, there).
		EXECUTE_PROCESS(
			COMMAND ${R_EXECUTABLE} "--slave" "--no-save" "--no-init-file" "-e" "cat(R.home('include'))"
			OUTPUT_VARIABLE R_INCLUDEDIR)
	ELSE(WIN32 OR APPLE)
		EXECUTE_PROCESS(
			COMMAND ${R_EXECUTABLE} CMD sh -c "echo -n $R_INCLUDE_DIR"
			OUTPUT_VARIABLE R_INCLUDEDIR)
	ENDIF(WIN32 OR APPLE)
ELSE(NOT R_INCLUDEDIR)
	MESSAGE(STATUS "Location specified by user")
ENDIF(NOT R_INCLUDEDIR)

IF(NOT R_INCLUDEDIR)
	SET(R_INCLUDEDIR ${R_HOME}/include)
	MESSAGE(STATUS "Not findable via R. Guessing")
ENDIF(NOT R_INCLUDEDIR)
MESSAGE(STATUS "Include files should be at ${R_INCLUDEDIR}. Checking for R.h")

FIND_FILE(R_H
	R.h
	PATHS ${R_INCLUDEDIR}
	NO_DEFAULT_PATH)
IF(NOT R_H)
	MESSAGE(FATAL_ERROR "Not found")
ELSE(NOT R_H)
	MESSAGE(STATUS "Found at ${R_H}")
	GET_FILENAME_COMPONENT(R_INCLUDEDIR ${R_H}
				PATH)
ENDIF(NOT R_H)
SET(R_INCLUDEDIR ${R_INCLUDEDIR} ${R_INCLUDEDIR}/${R_ARCH})

# check for existence of libR.so

MESSAGE(STATUS "Checking for existence of R shared library")
FIND_LIBRARY(LIBR_SO
	R
	PATHS ${R_HOME}/lib ${R_SHAREDLIBDIR} ${R_HOME}/bin ${R_HOME}/bin/${R_ARCH} ${R_HOME}/lib/${R_ARCH} ${PROJECT_BINARY_DIR}
	NO_DEFAULT_PATH)
IF(NOT LIBR_SO)
# NOTE: Workaround for strange bug with cmake 3.17 failing to find existing R.dll, when MinGW is used.
# Remove when cmake is working properly, again
FIND_FILE(LIBR_SO
	R.dll
	PATHS ${R_HOME}/lib ${R_SHAREDLIBDIR} ${R_HOME}/bin ${R_HOME}/bin/${R_ARCH} ${R_HOME}/lib/${R_ARCH} ${PROJECT_BINARY_DIR}
	NO_CMAKE_SYSTEM_PATH)
ENDIF(NOT LIBR_SO)
IF(NOT LIBR_SO)
	MESSAGE(FATAL_ERROR "Not found. Make sure the location of R was detected correctly, above, and R was compiled with the --enable-R-shlib option")
ELSE(NOT LIBR_SO)
	MESSAGE(STATUS "Exists at ${LIBR_SO}")
	GET_FILENAME_COMPONENT(R_SHAREDLIBDIR ${LIBR_SO}
				PATH)
	SET(R_USED_LIBS R)
ENDIF(NOT LIBR_SO)

# for at least some versions of R, we seem to have to link against -lRlapack. Else loading some
# R packages will fail due to unresolved symbols, or we can't link against -lR.
# However, we can't do this unconditionally,
# as this is not available in some configurations of R

MESSAGE(STATUS "Checking whether we should link against Rlapack library")
FIND_LIBRARY(R_LAPACK_LIBRARY Rlapack HINTS ${R_SHARED_LIB_DIR} )
IF(NOT R_LAPACK_LIBRARY)
	MESSAGE(STATUS "No, it does not exist in ${R_SHARED_LIB_DIR}")
ELSE(NOT R_LAPACK_LIBRARY)
	MESSAGE(STATUS "Yes, ${R_LAPACK_LIBRARY} exists")
	SET(R_LIBRARIES ${R_LIBRARIES} ${R_LAPACK_LIBRARY})
	IF(UNIX)
		# libgfortran is needed when linking to Rlapack on linux for some unknown reason.
		# apparently not needed on windows or Mac (let's see, when it comes back to bite us, though)
		# and compiling on windows is hard enough even without requiring libgfortran, too.
		# Query gfortran to get the libgfortran.so path
		FIND_PROGRAM(_GFORTRAN_EXECUTABLE NAMES gfortran)
		IF(_GFORTRAN_EXECUTABLE)
				EXECUTE_PROCESS(COMMAND ${_GFORTRAN_EXECUTABLE} -print-file-name=libgfortran.so
				OUTPUT_VARIABLE _libgfortran_path
				OUTPUT_STRIP_TRAILING_WHITESPACE
				)
		ENDIF()
		IF(EXISTS ${_libgfortran_path})
			SET(GFORTRAN_LIBRARY ${_libgfortran_path})
		ELSE()
			# if libgfortran wasn't found at this point, the installation is probably broken
			# Let's try to find the library nonetheless.
			FIND_LIBRARY(GFORTRAN_LIBRARY gfortran)
		ENDIF()
		IF (GFORTRAN_LIBRARY)
			SET(R_LIBRARIES ${R_LIBRARIES} ${GFORTRAN_LIBRARY})
		ELSE (GFORTRAN_LIBRARY)
			MESSAGE(STATUS "gfortran is needed for Rlapack but it could not be found")
			SET(ABORT_CONFIG TRUE)
		ENDIF (GFORTRAN_LIBRARY)
	ENDIF(UNIX)
ENDIF(NOT R_LAPACK_LIBRARY)

# for at least some versions of R, we seem to have to link against -lRlapack. Else loading some
# R packages will fail due to unresolved symbols, or we can't link against -lR.
# However, we can't do this unconditionally,
# as this is not available in some configurations of R

MESSAGE(STATUS "Checking whether we should link against Rblas library")
FIND_LIBRARY(LIBR_BLAS
	Rblas
	PATHS ${R_SHAREDLIBDIR}
	NO_DEFAULT_PATH)
IF(NOT LIBR_BLAS)
	MESSAGE(STATUS "No, it does not exist in ${R_SHAREDLIBDIR}")
ELSE(NOT LIBR_BLAS)
	MESSAGE(STATUS "Yes, ${LIBR_BLAS} exists")
	SET(R_USED_LIBS ${R_USED_LIBS} Rblas)
ENDIF(NOT LIBR_BLAS)
