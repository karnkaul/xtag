add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${PROJECT_NAME}::${PROJECT_NAME}-lib
  clap::clap
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE
  ${sources}
)

message(STATUS "[${package_name}] setting up install")
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME})
