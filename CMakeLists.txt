cmake_minimum_required(VERSION 3.10)

project(cstr)
set(CMAKE_C_STANDARD 11)

add_subdirectory(src) 

if(CMAKE_BUILD_TYPE MATCHES Debug)
  include(CTest)
  add_subdirectory(Unity) 
  add_subdirectory(test)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

