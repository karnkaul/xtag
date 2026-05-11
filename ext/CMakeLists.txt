file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT TARGET klib::klib)
  message(STATUS "[klib]")
  set(KLIB_INSTALL OFF)
  set(KLIB_UNIT_TEST ON)
  add_subdirectory(src/klib)
endif()

if(NOT TARGET djson::djson)
  message(STATUS "[djson]")
  set(DJ_INSTALL OFF)
  add_subdirectory(src/djson)
endif()

if(NOT TARGET clap::clap)
  message(STATUS "[clap]")
  add_subdirectory(src/clap)
endif()

if(NOT TARGET gvdi::gvdi)
  message(STATUS "[gvdi]")
  add_subdirectory(src/gvdi)
endif()

if(NOT TARGET ksignal::ksignal)
  message(STATUS "[ksignal]")
  add_subdirectory(src/ksignal)
endif()
