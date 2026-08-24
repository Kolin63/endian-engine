cmake_minimum_required(VERSION 3.16)

find_package(CURL REQUIRED)

add_library(concord STATIC IMPORTED)

set_target_properties(concord PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libdiscord.a"
)


project(reflector C)

file(GLOB REFLECTOR_SRC "mir/*.c" "dependencies/*.c" "src/fileio.c")

add_executable(reflector ${REFLECTOR_SRC})

target_link_libraries(reflector PRIVATE concord CURL::libcurl)

target_include_directories(reflector PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(reflector PROPERTIES
  C_STANDARD 23
  C_STANDARD_REQUIRED ON
)


project(server C)

file(GLOB SERVER_SRC "src/*.c" "dependencies/*.c" "ref/*.c")

add_executable(server ${SERVER_SRC})
add_compile_definitions(ENDIAN_ENGINE)

target_link_libraries(server PRIVATE concord CURL::libcurl)

target_include_directories(server PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies"
  "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(server PROPERTIES
  C_STANDARD 23
  C_STANDARD_REQUIRED ON
)
