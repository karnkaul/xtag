cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

include(version.cmake)

project(${package_name} VERSION ${build_version})

option(XTAG_BUILD_CLI "Build xtag CLI" ${PROJECT_IS_TOP_LEVEL})
option(XTAG_BUILD_GUI "Build xtag GUI" ${PROJECT_IS_TOP_LEVEL})
option(XTAG_BUILD_TESTS "Build xtag unit tests" ${PROJECT_IS_TOP_LEVEL})

set(is_linux OFF)
set(is_windows OFF)

if(WIN32)
  set(is_windows ON)
else()
  set(is_linux ON)
endif()

option(XTAG_USE_XATTR "Use extended attributes (Linux)" ${is_linux})
option(XTAG_USE_ADS "Use Alternate Data Streams (Windows)" ${is_windows})

if(NOT XTAG_USE_XATTR AND NOT XTAG_USE_ADS)
  message(FATAL_ERROR "One of XTAG_USE_XATTR or XTAG_USE_ADS must be enabled")
endif()

if(XTAG_USE_XATTR AND XTAG_USE_ADS)
  message(FATAL_ERROR "Only one of XTAG_USE_XATTR or XTAG_USE_ADS must be enabled")
endif()

add_subdirectory(ext)
add_subdirectory(lib)

if(XTAG_BUILD_CLI)
  add_subdirectory(cli)
endif()

if(XTAG_BUILD_GUI)
  add_subdirectory(gui)
endif()

if(XTAG_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
