include(ECMMarkAsTest)

find_package(Qt5 5.9 QUIET REQUIRED Test)

macro(rkward_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)

    target_link_libraries(${_testname} PRIVATE rkward_lib Qt5::Test)

    add_test(NAME rkward-${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

rkward_executable_tests(
  core_test
)
