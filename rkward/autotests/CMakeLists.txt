# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
include(ECMMarkAsTest)

find_package(Qt6 QUIET REQUIRED Test)

macro(rkward_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)

    target_link_libraries(${_testname} PRIVATE rkward_lib Qt6::Test)

    add_test(NAME rkward-${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

add_definitions(-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}//data/\")

rkward_executable_tests(
  core_test
  rkparsedscript_test
)
