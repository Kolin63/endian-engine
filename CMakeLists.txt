cmake_minimum_required(VERSION 3.16)

project(server C)

file(GLOB SOURCES "src/*.c" "dependencies/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
add_compile_definitions(ENDIAN_ENGINE)

find_package(CURL REQUIRED)

add_library(concord STATIC IMPORTED)

set_target_properties(concord PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libdiscord.a"
)

target_link_libraries(${PROJECT_NAME} PRIVATE concord CURL::libcurl)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD 23
  C_STANDARD_REQUIRED ON
)
