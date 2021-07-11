cmake_minimum_required(VERSION 3.10)

project(cstr)
set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/Modules/
)
include(GroupFiles)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  include(CTest)
  #include(CodeCoverage)
  #append_coverage_compiler_flags()
  #setup_target_for_coverage_lcov(
  #      NAME Coverage
  #      EXECUTABLE ctest
  #)
  set(GEN_UNIT_TESTS 1)
  add_subdirectory(Unity) 
  add_subdirectory(test)
endif(CMAKE_BUILD_TYPE MATCHES Debug)


add_subdirectory(src) 
