cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

include(version.cmake)

project(${package_name} VERSION ${build_version})

option(XTAG_BUILD_CLI "Build xtag CLI" ${PROJECT_IS_TOP_LEVEL})
option(XTAG_BUILD_TESTS "Build xtag unit tests" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ext)
add_subdirectory(lib)

if(XTAG_BUILD_CLI)
  add_subdirectory(cli)
endif()

if(XTAG_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
