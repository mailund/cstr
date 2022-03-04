cmake_minimum_required(VERSION 3.10)

project(cstr)
set(CMAKE_C_STANDARD 23)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/Modules/
)
include(GroupFiles)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)


if(CMAKE_BUILD_TYPE MATCHES Debug)
  include(CTest)
  # FIXME: need to fix it so Xcode gets the right flags for coverage.
  #include(CodeCoverage)
  #append_coverage_compiler_flags()
  #setup_target_for_coverage_lcov(
  #      NAME Coverage
  #      EXECUTABLE ctest
  #)
  set(GEN_UNIT_TESTS 1)
  add_subdirectory(test)
endif(CMAKE_BUILD_TYPE MATCHES Debug)


add_subdirectory(src) 
add_subdirectory(tools)

