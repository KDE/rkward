# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
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

rkward_executable_tests(
  core_test
)
